#include <complex>
#include <vector>
#include <cmath>
#include <GL/glew.h>
#include <GL/glut.h>
#include <stdio.h>
#include <chrono>
#include "glsl.h"
#include "MyGL.h"

#define PI 3.14159265358979323846

using namespace std;
using namespace std::chrono;

struct signalDeconstructed {
    complex<float> coeff;
    float phase;
    float freq;
    float range;
};

const int N = 170;
vector<complex<float>> signal(N);
vector<signalDeconstructed> dft_result(N);
vector<signalDeconstructed> cdft_result(N);

vector<complex<float>> draw_star_epicycle(N);

MyGL *gl = NULL;

float ang = 0;
float last_sample = 0;
float time_var = 0;
float sample_time = 0;
int contador = 0;
bool modo = 1;

steady_clock::time_point start_time;
steady_clock::time_point last_time;

void init_gl(void)
{
   glClearColor (1.0f, 1.0f, 1.0f, 0.0f);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void draw_circle(float x, float y, float radius) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 100; ++i) {
        float theta = 2.0 * M_PI * i / 100;
        float cx = x + radius * cos(theta);
        float cy = y + radius * sin(theta);
        glVertex2f(cx, cy);
    }
    glEnd();
}

void init_signal() {
    int npoints = 5;
    double radius1 = 0.5;
    double radius2 = 1.0;
    double angle = 2.0 * M_PI / (npoints * 2);

    vector<complex<float>> main_vertices(10);

    for (int i = 0; i < 10; ++i) {
        double a = i * angle;
        if (i % 2 == 0) {
            main_vertices[i] = complex<float>(cos(a) * radius2, sin(a) * radius2);
        } else {
            main_vertices[i] = complex<float>(cos(a) * radius1, sin(a) * radius1);
        }
    }

    for (int i = 0; i < 10; ++i) {
        int next = (i + 1) % 10;
        for (int j = 0; j < N / 10; ++j) {
            float t = static_cast<float>(j) / (N / 10 - 1);
            signal[i * (N / 10) + j] = main_vertices[i] * (1.0f - t) + main_vertices[next] * t;
        }
    }
}


void init_star_epicycle() {
    for (int i = 0; i < N; ++i) {
        draw_star_epicycle[i] = complex<float>(0.0f, 0.0f);
    }
}

void init_dft() {
    for (int k = 0; k < N; ++k) {
        dft_result[k].coeff = complex<float>(0.0f, 0.0f);
        dft_result[k].phase = 0.0f;
        dft_result[k].freq = 0.0f;
        dft_result[k].range = 0.0f;
    }
}

void init_cdft() {
    for (int k = 0; k < N; ++k) {
        cdft_result[k].coeff = complex<float>(0.0f, 0.0f);
        cdft_result[k].phase = 0.0f;
        cdft_result[k].freq = 0.0f;
        cdft_result[k].range = 0.0f;
    }
}

void compute_dft() {
    for (int k = 0; k < N; ++k) {
        complex<float> sum(0.0f, 0.0f);
        for (int n = 0; n < N; ++n) {
            float theta = 2.0 * M_PI * k * n / N;
            complex<float> c = complex<float>(cos(theta), -sin(theta));
            sum += signal[n] * c;
        }
        dft_result[k].coeff = sum / static_cast<float>(N);
        dft_result[k].phase = atan2(dft_result[k].coeff.imag(), dft_result[k].coeff.real());
        dft_result[k].freq = k;
        dft_result[k].range = abs(dft_result[k].coeff);
    }
}

void compute_cdft() {
    for (int k = 0; k < N; ++k) {
        complex<float> sum(0.0f, 0.0f);
        for (int n = 0; n < N; ++n) {
            double theta = -2.0 * M_PI * k * n / N;
            float real_part = signal[n].real() * cos(theta) - signal[n].imag() * sin(theta);
            float imag_part = signal[n].real() * sin(theta) + signal[n].imag() * cos(theta);
            sum += complex<float>(real_part, imag_part);
        }
        cdft_result[k].coeff = sum / static_cast<float>(N);
        cdft_result[k].phase = atan2(cdft_result[k].coeff.imag(), cdft_result[k].coeff.real());
        cdft_result[k].freq = k;
        cdft_result[k].range = abs(cdft_result[k].coeff);
    }
}


void draw_star() {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < signal.size(); ++i) {
        glColor3f(0, 1, 0); 
        glVertex2f(signal[i].real(), signal[i].imag());
    }
    glEnd();

    /*
    glColor3f(0, 0, 0); 
    for (int i = 0; i < signal.size(); ++i) {
        draw_circle(signal[i].real(), signal[i].imag(), 0.05);
    }*/
}

void draw_epicylcle_star() {
    glBegin(GL_LINES);
    for (int i = 0; i < contador - 1; ++i) {
        glVertex2d(draw_star_epicycle[i].real(), draw_star_epicycle[i].imag());
        glVertex2d(draw_star_epicycle[i + 1].real(), draw_star_epicycle[i + 1].imag());
    }
    glEnd();
}


void draw_epicycles(const vector<signalDeconstructed>& coeffs, double time, int atualiza, float x = 0, float y = 0, float rotation = 0) {
    float prevX, prevY;

    for (int i = 0; i < coeffs.size(); i++) {
        prevX = x;
        prevY = y;
        float freq = coeffs[i].freq;
        float phase = coeffs[i].phase;
        float amp = coeffs[i].range;

        x += amp * cos(freq * time + phase + rotation);
        y += amp * sin(freq * time + phase + rotation);

        glBegin(GL_LINE_LOOP);
        for (int j = 0; j < 100; ++j) {
            float theta = 2.0 * PI * j / 100;
            float cx = prevX + amp * cos(theta);
            float cy = prevY + amp * sin(theta);
            glColor3f(0, 0, 0);
            glVertex2f(cx, cy);
        }
        glEnd();

        glBegin(GL_LINES);
        glColor3f(0, 0, 0);
        glVertex2f(prevX, prevY);
        glVertex2f(x, y);
        glEnd();

        if (atualiza == 1) {
            if (i == coeffs.size() - 1) {
                draw_star_epicycle[contador] = complex<float>(x, y);
                contador += 1;
                if (contador == N) {
                    contador = 0;
                }
            }
        }
    }
}


void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gl->matrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gl->loadIdentity();

    steady_clock::time_point current_time = steady_clock::now();
    duration<float> elapsed_time = duration_cast<duration<float>>(current_time - start_time);
    start_time = current_time;
    double current_time_in_seconds = elapsed_time.count();

    time_var += current_time_in_seconds * 0.1;

    double sample_delay = 2 * PI / N;

    if (time_var > last_sample + sample_delay) {
        sample_time += sample_delay;
        last_sample = sample_time;

        if (modo) {
            draw_epicycles(dft_result, sample_time, 1);
        } else {
            draw_epicycles(cdft_result, sample_time, 1);
        }
    }

    draw_star();

    if (modo) {
        draw_epicycles(dft_result, time_var, 0);
        glColor3f(1, 0, 0);
        draw_epicylcle_star();
    } else {
        draw_epicycles(cdft_result, time_var, 0);
        glColor3f(0, 0, 1);
        draw_epicylcle_star();
    }

    glutSwapBuffers();
}


void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float aspect_ratio = (float)w / (float)h;
    float scale = 1.0; 

    if (w <= h) {
        glOrtho(-scale, scale, -scale / aspect_ratio, scale / aspect_ratio, -100.0, 100.0);
    } else {
        glOrtho(-scale * aspect_ratio, scale * aspect_ratio, -scale, scale, -100.0, 100.0);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27:
            exit(0);
            break;
        case 'c':
            modo = !modo;
            draw_star_epicycle.clear();
            break;
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Rafael Carneiro Pregardier");
    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);

    init_gl();

    gl = new MyGL();

    init_signal();
    init_dft();
    init_star_epicycle();
    compute_dft();
    compute_cdft();

    start_time = steady_clock::now();
    glutMainLoop();
    return 0;
}

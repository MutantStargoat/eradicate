#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <GL/glut.h>
#include "curve.h"
#include "track.h"

static int init(void);
static void cleanup(void);
static void display(void);
static void draw_curve(struct curve *c);
static void draw_grid(void);
static void reshape(int x, int y);
static void keyb(unsigned char key, int x, int y);
static void keyb_up(unsigned char key, int x, int y);
static void mouse(int bn, int st, int x, int y);
static void motion(int x, int y);

static int parse_args(int argc, char **argv);


static float cam_theta, cam_phi = 10, cam_dist = 10;
static cgm_vec3 cam_pos;
static int mouse_x, mouse_y;
static int bnstate[8];
static unsigned char keystate[256];
static unsigned int modkeys;

static struct curve *curve;
static float cpos_t;
static cgm_vec3 cpos;

static int seg_subdiv = 4;
static struct track *trk;


int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(800, 600);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutCreateWindow("tracktool");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyb);
	glutKeyboardUpFunc(keyb_up);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	if(parse_args(argc, argv) == -1) {
		return 1;
	}

	if(init() == -1) {
		return 1;
	}
	atexit(cleanup);

	glutMainLoop();
	return 0;
}

static int init(void)
{
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	return 0;
}

static void cleanup(void)
{
}

static void display(void)
{
	glClearColor(0.05, 0.05, 0.05, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -cam_dist);
	glRotatef(cam_phi, 1, 0, 0);
	glRotatef(cam_theta, 0, 1, 0);
	glTranslatef(-cam_pos.x, -cam_pos.y, -cam_pos.z);

	draw_grid();
	draw_curve(curve);

	glutSwapBuffers();
	assert(glGetError() == GL_NO_ERROR);
}

static void draw_curve(struct curve *c)
{
	int i, num_seg = c->num_cp * 16;
	float t;
	cgm_vec3 p;

	glBegin(GL_LINE_STRIP);
	glColor3f(0, 1, 0);
	for(i=0; i<num_seg; i++) {
		t = (float)i / (float)(num_seg - 1);
		eval_curve(c, t, &p);

		glVertex3f(p.x, p.y, p.z);
	}
	glEnd();

/*
	glPushAttrib(GL_ENABLE_BIT | GL_POINT_BIT);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(5.0);
	glBegin(GL_POINTS);
	glColor3f(1, 0, 0);
	glVertex3f(cpos.x, cpos.y, cpos.z);
	glEnd();
	glPopAttrib();
*/
}

static void draw_grid()
{
	glLineWidth(2);
	glBegin(GL_LINES);
	glColor3f(0.7, 0, 0);
	glVertex4f(-1, 0, 0, 0);
	glVertex4f(0, 0, 0, 1);
	glVertex4f(0, 0, 0, 1);
	glVertex4f(1, 0, 0, 0);
	glColor3f(0, 0, 0.7);
	glVertex4f(0, 0, -1, 0);
	glVertex4f(0, 0, 0, 1);
	glVertex4f(0, 0, 0, 1);
	glVertex4f(0, 0, 1, 0);
	glEnd();
}

static void reshape(int x, int y)
{
	glViewport(0, 0, x, y);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(50.0, (float)x / (float)y, 0.5, 10000.0);
}

static void keyb(unsigned char key, int x, int y)
{
	switch(key) {
	case 27:
		exit(0);

	case 'g':
		if(!trk) {
			if(!(trk = malloc(sizeof *trk))) {
				perror("failed to allocate track");
				break;
			}
			if(create_track(trk, curve) == -1) {
				free(trk);
				trk = 0;
				break;
			}
		}
		printf("generating track mesh with %d subdivisions per segment\n", seg_subdiv);
		gen_track_mesh(trk, seg_subdiv);
		break;
	}

	keystate[key] = 1;
	modkeys = glutGetModifiers();
}

static void keyb_up(unsigned char key, int x, int y)
{
	keystate[key] = 0;
}

static void mouse(int bn, int st, int x, int y)
{
	int bidx = bn - GLUT_LEFT_BUTTON;
	if(bidx < sizeof bnstate / sizeof *bnstate) {
		bnstate[bidx] = st == GLUT_DOWN;
	}
	mouse_x = x;
	mouse_y = y;
	modkeys = glutGetModifiers();
}

static void motion(int x, int y)
{
	int dx = x - mouse_x;
	int dy = y - mouse_y;
	mouse_x = x;
	mouse_y = y;

	if((dx | dy) == 0) return;

	/*
	if(keystate['t']) {
		cpos_t += dx * 0.001;
		if(cpos_t < 0) cpos_t = 0;
		if(cpos_t > 1) cpos_t = 1;
		eval_curve(curve, cpos_t, &cpos);
		printf("t: %f -> (%g %g %g)\n", cpos_t, cpos.x, cpos.y, cpos.z);
		glutPostRedisplay();
		return;
	}
	*/

	if(bnstate[0]) {
		cam_theta += dx * 0.5;
		cam_phi += dy * 0.5;
		if(cam_phi < -90) cam_phi = -90;
		if(cam_phi > 90) cam_phi = 90;
		glutPostRedisplay();
	}
	if(bnstate[1]) {
		float theta = cgm_deg_to_rad(cam_theta);
		cgm_vec3 fwd, right;

		cgm_vcons(&right, cos(theta), 0, sin(theta));
		cgm_vcons(&fwd, -sin(theta), 0, cos(theta));

		cgm_vadd_scaled(&cam_pos, &right, dx * -0.1 * log(cam_dist));
		cgm_vadd_scaled(&cam_pos, &fwd, dy * -0.1 * log(cam_dist));
		glutPostRedisplay();
	}
	if(bnstate[2]) {
		if(modkeys & GLUT_ACTIVE_CTRL) {
			cam_dist += dy * 0.1 / cam_dist;
		} else {
			cam_dist += dy * 0.01 * cam_dist;
		}
		if(cam_dist < 0.1) cam_dist = 0.1;
		if(cam_dist > 10000) cam_dist = 10000;
		glutPostRedisplay();
	}
}

static int parse_args(int argc, char **argv)
{
	int i;
	cgm_vec3 bbmin, bbmax, bbrad;
	float rad;

	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
			if(argv[i][2] == 0) {
				switch(argv[i][1]) {
				case 'c':
					if(!(curve = load_curve(argv[++i]))) {
						return -1;
					}
					curve_bounds(curve, &bbmin, &bbmax);
					bbrad.x = (bbmax.x - bbmin.x) / 2.0f;
					bbrad.y = (bbmax.y - bbmin.y) / 2.0f;
					bbrad.z = (bbmax.z - bbmin.z) / 2.0f;
					rad = bbrad.x > bbrad.y ? (bbrad.x > bbrad.z ? bbrad.x : bbrad.z) : bbrad.y;
					cam_dist = rad / tan(25.0 * M_PI / 180.0);
					cam_pos.x = (bbmax.x + bbmin.x) / 2.0f;
					cam_pos.y = (bbmax.y + bbmin.y) / 2.0f;
					cam_pos.z = (bbmax.z + bbmin.z) / 2.0f;
					break;

				default:
					fprintf(stderr, "invalid option: %s\n", argv[i]);
					return -1;
				}
			} else {
				fprintf(stderr, "invalid option: %s\n", argv[i]);
				return -1;
			}
		} else {
			fprintf(stderr, "unexpected argument: %s\n", argv[i]);
			return -1;
		}
	}

	if(!curve) {
		fprintf(stderr, "please pass a curve file (-c <curve_filename>)\n");
		return -1;
	}

	return 0;
}

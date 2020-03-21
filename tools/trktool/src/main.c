#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>
#include <GL/glut.h>
#include "curve.h"
#include "track.h"

static int init(void);
static void cleanup(void);
static void display(void);
static void draw_track(struct track *trk);
static void draw_curve(struct curve *c);
static void draw_grid(void);
static void scr_printf(int x, int y, const char *s, ...);
static void reshape(int x, int y);
static void keyb(unsigned char key, int x, int y);
static void keyb_up(unsigned char key, int x, int y);
static void mouse(int bn, int st, int x, int y);
static void motion(int x, int y);

static void generate(void);
static void follow(void);

static int parse_args(int argc, char **argv);


static int win_width, win_height;
static float win_aspect;
static float cam_theta, cam_phi = 10, cam_dist = 10;
static cgm_vec3 cam_pos;
static int mouse_x, mouse_y;
static int bnstate[8];
static unsigned char keystate[256];
static unsigned int modkeys;

static struct curve *curve;
static float cpos_t;
static cgm_vec3 cpos, cdir;

static int wireframe;
static int follow_cam;

static int seg_subdiv = 18;
static float twist = 30.0f;
static struct track *trk;


int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(1280, 800);
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

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	follow();	/* run the camera follow routine once to initialize all variables */
	return 0;
}

static void cleanup(void)
{
}

static void display(void)
{
	if(follow_cam) {
		cam_pos = cpos;
		cam_theta = cgm_rad_to_deg(atan2(cdir.x, -cdir.z));
	}

	glClearColor(0.05, 0.05, 0.05, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -cam_dist);
	glRotatef(cam_phi, 1, 0, 0);
	glRotatef(cam_theta, 0, 1, 0);
	glTranslatef(-cam_pos.x, -cam_pos.y, -cam_pos.z);

	glDisable(GL_LIGHTING);
	draw_grid();
	draw_curve(curve);

	if(trk) {
		if(!wireframe) glEnable(GL_LIGHTING);
		draw_track(trk);
	}

	glColor3f(0.6, 1.0, 0.6);
	scr_printf(10, 20, "camera: %s  (tab)", follow_cam ? "follow" : "orbit");
	scr_printf(10, 40, "segment subdiv: %d  (+/-)", seg_subdiv);
	scr_printf(10, 60, "twist factor: %g  ([/])", twist);

	glutSwapBuffers();
	assert(glGetError() == GL_NO_ERROR);
}

static void draw_track(struct track *trk)
{
	int i;
	struct track_segment *tseg = trk->tseg;

	glShadeModel(GL_FLAT);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	for(i=0; i<trk->num_tseg; i++) {
		glVertexPointer(3, GL_FLOAT, sizeof(struct g3d_vertex), &tseg->mesh.varr->x);
		glNormalPointer(GL_FLOAT, sizeof(struct g3d_vertex), &tseg->mesh.varr->nx);
		glTexCoordPointer(2, GL_FLOAT, sizeof(struct g3d_vertex), &tseg->mesh.varr->u);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(struct g3d_vertex), &tseg->mesh.varr->r);

		glDrawElements(GL_QUADS, tseg->mesh.icount, GL_UNSIGNED_SHORT, tseg->mesh.iarr);
		tseg++;
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glShadeModel(GL_SMOOTH);
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

	glPushAttrib(GL_ENABLE_BIT | GL_POINT_BIT);
	glEnable(GL_POINT_SMOOTH);
	glDisable(GL_LIGHTING);
	glPointSize(5.0);
	glBegin(GL_POINTS);
	glColor3f(1, 0, 0);
	glVertex3f(cpos.x, cpos.y, cpos.z);
	glEnd();
	glPopAttrib();
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
	glLineWidth(1);
}

static void scr_printf(int x, int y, const char *s, ...)
{
	char buf[1024];
	va_list ap;

	va_start(ap, s);
	vsprintf(buf, s, ap);
	va_end(ap);
	s = buf;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, win_width, win_height, 0, -1, 1);

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glRasterPos2i(x, y);
	while(*s) {
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *s++);
	}

	glPopAttrib();
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

static void reshape(int x, int y)
{
	win_width = x;
	win_height = y;
	win_aspect = (float)x / (float)y;
	glViewport(0, 0, x, y);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(50.0, win_aspect, 0.5, 10000.0);
}

static void keyb(unsigned char key, int x, int y)
{
	switch(key) {
	case 27:
		exit(0);

	case '\t':
		follow_cam = !follow_cam;
		glutPostRedisplay();
		break;

	case 'g':
		generate();
		glutPostRedisplay();
		break;

	case '=':
		seg_subdiv++;
		generate();
		glutPostRedisplay();
		break;

	case '-':
		seg_subdiv--;
		generate();
		glutPostRedisplay();
		break;

	case ']':
		twist += 5.0f;
		generate();
		glutPostRedisplay();
		break;

	case '[':
		twist -= 5.0f;
		generate();
		glutPostRedisplay();
		break;

	case 'w':
		wireframe = !wireframe;
		glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
		glutPostRedisplay();
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

	if(keystate['f']) {
		cpos_t += dx * 0.001;
		cpos_t = fmod(cpos_t, 1.0);
		if(cpos_t < 0.0f) cpos_t += 1.0f;

		follow();
		glutPostRedisplay();
		return;
	}

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

static void generate(void)
{
	if(!trk) {
		if(!(trk = malloc(sizeof *trk))) {
			perror("failed to allocate track");
			return;
		}
		if(create_track(trk, curve) == -1) {
			free(trk);
			trk = 0;
			return;
		}
	}
	printf("generating track mesh with %d subdivisions per segment\n", seg_subdiv);
	gen_track_mesh(trk, seg_subdiv, twist);
}

static void follow(void)
{
	eval_curve(curve, cpos_t, &cpos);
	eval_tangent(curve, cpos_t, &cdir);
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
					curve->mode = CURVE_REPEAT;
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

	generate();
	return 0;
}

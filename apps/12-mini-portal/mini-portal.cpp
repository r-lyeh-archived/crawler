/**
 * From the OpenGL Programming wikibook: http://en.wikibooks.org/wiki/OpenGL_Programming
 * This file is in the public domain.
 * Contributors: Sylvain Beucler
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//#include <unistd.h>
#include <windows.h>
#define sleep(x) Sleep((x)*1000)

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
/* Use glew.h instead of gl.h to get all the GL prototypes declared */
#include <GL/glew.h>
/* Using the GLUT library for the base windowing setup */
#include <GL/glut.h>
/* GLM */
// #define GLM_MESSAGES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "common/shader_utils.h"

#define GROUND_SIZE 20

int screen_width=800, screen_height=600;
GLuint program;
GLint attribute_v_coord;
GLint attribute_v_normal;
GLint uniform_m, uniform_v, uniform_p;
GLint uniform_m_3x3_inv_transp, uniform_v_inv;
bool compute_arcball;
int last_mx = 0, last_my = 0, cur_mx = 0, cur_my = 0;
int arcball_on = false;

using namespace std;

struct rect { int x,y,w,h; };

enum MODES { MODE_OBJECT, MODE_CAMERA, MODE_LIGHT, MODE_LAST } view_mode = MODE_CAMERA;
int rotY_direction = 0, rotX_direction = 0, transZ_direction = 0, strife = 0;
float speed_factor = 1;
glm::mat4 transforms[MODE_LAST];
int last_ticks = 0;

static unsigned int fps_start = 0;
static unsigned int fps_frames = 0;

static float zNear = 0.01;
static float fovy = 45;

class Mesh {
private:
  GLuint vbo_vertices, vbo_normals, ibo_elements;
public:
  vector<glm::vec4> vertices;
  vector<glm::vec3> normals;
  vector<GLushort> elements;
  glm::mat4 object2world;

  Mesh() : vbo_vertices(0), vbo_normals(0), ibo_elements(0), object2world(glm::mat4(1)) {}
  ~Mesh() {
    if (vbo_vertices != 0)
      glDeleteBuffers(1, &vbo_vertices);
    if (vbo_normals != 0)
      glDeleteBuffers(1, &vbo_normals);
    if (ibo_elements != 0)
      glDeleteBuffers(1, &ibo_elements);
  }

  /**
   * Store object vertices, normals and/or elements in graphic card
   * buffers
   */
  void upload() {
    if (this->vertices.size() > 0) {
      glGenBuffers(1, &this->vbo_vertices);
      glBindBuffer(GL_ARRAY_BUFFER, this->vbo_vertices);
      glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(this->vertices[0]),
		   this->vertices.data(), GL_STATIC_DRAW);
    }
    
    if (this->normals.size() > 0) {
      glGenBuffers(1, &this->vbo_normals);
      glBindBuffer(GL_ARRAY_BUFFER, this->vbo_normals);
      glBufferData(GL_ARRAY_BUFFER, this->normals.size() * sizeof(this->normals[0]),
		   this->normals.data(), GL_STATIC_DRAW);
    }
    
    if (this->elements.size() > 0) {
      glGenBuffers(1, &this->ibo_elements);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo_elements);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->elements.size() * sizeof(this->elements[0]),
		   this->elements.data(), GL_STATIC_DRAW);
    }
  }

  /**
   * Draw the object
   */
  void draw() {
    if (this->vbo_vertices != 0) {
      glEnableVertexAttribArray(attribute_v_coord);
      glBindBuffer(GL_ARRAY_BUFFER, this->vbo_vertices);
      glVertexAttribPointer(
        attribute_v_coord,  // attribute
        4,                  // number of elements per vertex, here (x,y,z,w)
        GL_FLOAT,           // the type of each element
        GL_FALSE,           // take our values as-is
        0,                  // no extra data between each position
        0                   // offset of first element
      );
    }

    if (this->vbo_normals != 0) {
      glEnableVertexAttribArray(attribute_v_normal);
      glBindBuffer(GL_ARRAY_BUFFER, this->vbo_normals);
      glVertexAttribPointer(
        attribute_v_normal, // attribute
        3,                  // number of elements per vertex, here (x,y,z)
        GL_FLOAT,           // the type of each element
        GL_FALSE,           // take our values as-is
        0,                  // no extra data between each position
        0                   // offset of first element
      );
    }
    
    /* Apply object's transformation matrix */
    glUniformMatrix4fv(uniform_m, 1, GL_FALSE, glm::value_ptr(this->object2world));
    glm::mat3 m_3x3_inv_transp = glm::transpose(glm::inverse(glm::mat3(this->object2world)));
    glUniformMatrix3fv(uniform_m_3x3_inv_transp, 1, GL_FALSE, glm::value_ptr(m_3x3_inv_transp));
    
    /* Push each element in buffer_vertices to the vertex shader */
    if (this->ibo_elements != 0) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo_elements);
      int size;  glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
      glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
    } else {
      glDrawArrays(GL_TRIANGLES, 0, this->vertices.size());
    }

    if (this->vbo_normals != 0)
      glDisableVertexAttribArray(attribute_v_coord);
    if (this->vbo_vertices != 0)
      glDisableVertexAttribArray(attribute_v_normal);
  }

  /**
   * Draw object bounding box
   */
  void draw_bbox() {
    if (this->vertices.size() == 0)
      return;
    
    // Cube 1x1x1, centered on origin
    GLfloat vertices[] = {
      -0.5, -0.5, -0.5, 1.0,
       0.5, -0.5, -0.5, 1.0,
       0.5,  0.5, -0.5, 1.0,
      -0.5,  0.5, -0.5, 1.0,
      -0.5, -0.5,  0.5, 1.0,
       0.5, -0.5,  0.5, 1.0,
       0.5,  0.5,  0.5, 1.0,
      -0.5,  0.5,  0.5, 1.0,
    };
    GLuint vbo_vertices;
    glGenBuffers(1, &vbo_vertices);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLushort elements[] = {
      0, 1, 2, 3,
      4, 5, 6, 7,
      0, 4, 1, 5, 2, 6, 3, 7
    };
    GLuint ibo_elements;
    glGenBuffers(1, &ibo_elements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_elements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    GLfloat
      min_x, max_x,
      min_y, max_y,
      min_z, max_z;
    min_x = max_x = this->vertices[0].x;
    min_y = max_y = this->vertices[0].y;
    min_z = max_z = this->vertices[0].z;
    for (unsigned int i = 0; i < this->vertices.size(); i++) {
      if (this->vertices[i].x < min_x) min_x = this->vertices[i].x;
      if (this->vertices[i].x > max_x) max_x = this->vertices[i].x;
      if (this->vertices[i].y < min_y) min_y = this->vertices[i].y;
      if (this->vertices[i].y > max_y) max_y = this->vertices[i].y;
      if (this->vertices[i].z < min_z) min_z = this->vertices[i].z;
      if (this->vertices[i].z > max_z) max_z = this->vertices[i].z;
    }
    glm::vec3 size = glm::vec3(max_x-min_x, max_y-min_y, max_z-min_z);
    glm::vec3 center = glm::vec3((min_x+max_x)/2, (min_y+max_y)/2, (min_z+max_z)/2);
    glm::mat4 transform = glm::scale(glm::mat4(1), size) * glm::translate(glm::mat4(1), center);
    
    /* Apply object's transformation matrix */
    glm::mat4 m = this->object2world * transform;
    glUniformMatrix4fv(uniform_m, 1, GL_FALSE, glm::value_ptr(m));
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
    glEnableVertexAttribArray(attribute_v_coord);
    glVertexAttribPointer(
      attribute_v_coord,  // attribute
      4,                  // number of elements per vertex, here (x,y,z,w)
      GL_FLOAT,           // the type of each element
      GL_FALSE,           // take our values as-is
      0,                  // no extra data between each position
      0                   // offset of first element
    );
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_elements);
    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, 0);
    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, (GLvoid*)(4*sizeof(GLushort)));
    glDrawElements(GL_LINES, 8, GL_UNSIGNED_SHORT, (GLvoid*)(8*sizeof(GLushort)));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    glDisableVertexAttribArray(attribute_v_coord);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glDeleteBuffers(1, &vbo_vertices);
    glDeleteBuffers(1, &ibo_elements);
  }
};
Mesh ground, main_object, light_bbox;
Mesh portals[2];


void draw_scene(vector<glm::mat4> view_stack, int rec, int outer_portal);
void draw_portals(vector<glm::mat4> view_stack, int rec, int outer_portal);

void load_obj(const char* filename, Mesh* mesh) {
  ifstream in(filename, ios::in);
  if (!in) { cerr << "Cannot open " << filename << endl; exit(1); }
  vector<int> nb_seen;

  string line;
  while (getline(in, line)) {
    if (line.substr(0,2) == "v ") {
      istringstream s(line.substr(2));
      glm::vec4 v; s >> v.x; s >> v.y; s >> v.z; v.w = 1.0;
      mesh->vertices.push_back(v);
    }  else if (line.substr(0,2) == "f ") {
      istringstream s(line.substr(2));
      GLushort a,b,c;
      s >> a; s >> b; s >> c;
      a--; b--; c--;
      mesh->elements.push_back(a); mesh->elements.push_back(b); mesh->elements.push_back(c);
    }
    else if (line[0] == '#') { /* ignoring this line */ }
    else { /* ignoring this line */ }
  }

  mesh->normals.resize(mesh->vertices.size(), glm::vec3(0.0, 0.0, 0.0));
  nb_seen.resize(mesh->vertices.size(), 0);
  for (unsigned int i = 0; i < mesh->elements.size(); i+=3) {
    GLushort ia = mesh->elements[i];
    GLushort ib = mesh->elements[i+1];
    GLushort ic = mesh->elements[i+2];
    glm::vec3 normal = glm::normalize(glm::cross(
      glm::vec3(mesh->vertices[ib]) - glm::vec3(mesh->vertices[ia]),
      glm::vec3(mesh->vertices[ic]) - glm::vec3(mesh->vertices[ia])));

    int v[3];  v[0] = ia;  v[1] = ib;  v[2] = ic;
    for (int j = 0; j < 3; j++) {
      GLushort cur_v = v[j];
      nb_seen[cur_v]++;
      if (nb_seen[cur_v] == 1) {
	mesh->normals[cur_v] = normal;
      } else {
	// average
	mesh->normals[cur_v].x = mesh->normals[cur_v].x * (1.0 - 1.0/nb_seen[cur_v]) + normal.x * 1.0/nb_seen[cur_v];
	mesh->normals[cur_v].y = mesh->normals[cur_v].y * (1.0 - 1.0/nb_seen[cur_v]) + normal.y * 1.0/nb_seen[cur_v];
	mesh->normals[cur_v].z = mesh->normals[cur_v].z * (1.0 - 1.0/nb_seen[cur_v]) + normal.z * 1.0/nb_seen[cur_v];
	mesh->normals[cur_v] = glm::normalize(mesh->normals[cur_v]);
      }
    }
  }
}

void create_portal(Mesh* portal, int screen_width, int screen_height, float zNear, float fovy) {
  portal->vertices.clear();
  portal->elements.clear();

  float aspect = 1.0 * screen_width / screen_height;
  float fovy_rad = fovy * M_PI / 180;
  float fovx_rad = fovy_rad / aspect;
  float dz = max(zNear/cos(fovx_rad), zNear/cos(fovy_rad));
  float dx = tan(fovx_rad) * dz;
  float dy = tan(fovy_rad) * dz;
  glm::vec4 portal_vertices[] = {
    glm::vec4(-1, -1, 0, 1),
    glm::vec4( 1, -1, 0, 1),
    glm::vec4(-1,  1, 0, 1),
    glm::vec4( 1,  1, 0, 1),

    glm::vec4(-(1+dx), -(1+dy), 0-dz, 1),
    glm::vec4( (1+dx), -(1+dy), 0-dz, 1),
    glm::vec4(-(1+dx),  (1+dy), 0-dz, 1),
    glm::vec4( (1+dx),  (1+dy), 0-dz, 1),
  };
  for (unsigned int i = 0; i < sizeof(portal_vertices)/sizeof(portal_vertices[0]); i++) {
    portal->vertices.push_back(portal_vertices[i]);
  }
  GLushort portal_elements[] = {
    0,1,2, 2,1,3,
    4,5,6, 6,5,7,
    0,4,2, 2,4,6,
    5,1,7, 7,1,3,
  };
  for (unsigned int i = 0; i < sizeof(portal_elements)/sizeof(portal_elements[0]); i++) {
    portal->elements.push_back(portal_elements[i]);
  }
}

int init_resources(char* model_filename, char* vshader_filename, char* fshader_filename)
{
  load_obj(model_filename, &main_object);
  // mesh position initialized in init_view()

  for (int i = -GROUND_SIZE/2; i < GROUND_SIZE/2; i++) {
    for (int j = -GROUND_SIZE/2; j < GROUND_SIZE/2; j++) {
      ground.vertices.push_back(glm::vec4(i,   0.0,  j+1, 1.0));
      ground.vertices.push_back(glm::vec4(i+1, 0.0,  j+1, 1.0));
      ground.vertices.push_back(glm::vec4(i,   0.0,  j,   1.0));
      ground.vertices.push_back(glm::vec4(i,   0.0,  j,   1.0));
      ground.vertices.push_back(glm::vec4(i+1, 0.0,  j+1, 1.0));
      ground.vertices.push_back(glm::vec4(i+1, 0.0,  j,   1.0));
      for (int k = 0; k < 6; k++)
	ground.normals.push_back(glm::vec3(0.0, 1.0, 0.0));
    }
  }

  glm::vec3 light_position = glm::vec3(0.0,  1.0,  2.0);
  light_bbox.vertices.push_back(glm::vec4(-0.1, -0.1, -0.1, 0.0));
  light_bbox.vertices.push_back(glm::vec4( 0.1, -0.1, -0.1, 0.0));
  light_bbox.vertices.push_back(glm::vec4( 0.1,  0.1, -0.1, 0.0));
  light_bbox.vertices.push_back(glm::vec4(-0.1,  0.1, -0.1, 0.0));
  light_bbox.vertices.push_back(glm::vec4(-0.1, -0.1,  0.1, 0.0));
  light_bbox.vertices.push_back(glm::vec4( 0.1, -0.1,  0.1, 0.0));
  light_bbox.vertices.push_back(glm::vec4( 0.1,  0.1,  0.1, 0.0));
  light_bbox.vertices.push_back(glm::vec4(-0.1,  0.1,  0.1, 0.0));
  light_bbox.object2world = glm::translate(glm::mat4(1), light_position);

  create_portal(&portals[0], screen_width, screen_height, zNear, fovy);
  create_portal(&portals[1], screen_width, screen_height, zNear, fovy);

  // Face to face
  // portals[0].object2world = glm::translate(glm::mat4(1), glm::vec3(-2, 0, 0))
  //   * glm::translate(glm::mat4(1), glm::vec3(0, 1, 0))
  //   * glm::rotate(glm::mat4(1), 90.0f, glm::vec3(0, 1, 0));
  // portals[1].object2world = glm::translate(glm::mat4(1), glm::vec3(2, 0, 0))
  //   * glm::translate(glm::mat4(1), glm::vec3(0, 1, 0))
  //   * glm::rotate(glm::mat4(1), -90.0f, glm::vec3(0, 1, 0));

  // 90° angle + slightly higher
  portals[0].object2world = glm::translate(glm::mat4(1), glm::vec3(0, 1, -2));
  portals[1].object2world = glm::rotate(glm::mat4(1), -90.0f, glm::vec3(0, 1, 0))
    * glm::translate(glm::mat4(1), glm::vec3(0, 1.2, -2));

  main_object.upload();
  ground.upload();
  light_bbox.upload();
  portals[0].upload();
  portals[1].upload();


  /* Compile and link shaders */
  GLint link_ok = GL_FALSE;
  GLint validate_ok = GL_FALSE;

  GLuint vs, fs;
  if ((vs = create_shader(vshader_filename, GL_VERTEX_SHADER))   == 0) return 0;
  if ((fs = create_shader(fshader_filename, GL_FRAGMENT_SHADER)) == 0) return 0;

  program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
  if (!link_ok) {
    fprintf(stderr, "glLinkProgram:");
    print_log(program);
    return 0;
  }
  glValidateProgram(program);
  glGetProgramiv(program, GL_VALIDATE_STATUS, &validate_ok);
  if (!validate_ok) {
    fprintf(stderr, "glValidateProgram:");
    print_log(program);
  }

  const char* attribute_name;
  attribute_name = "v_coord";
  attribute_v_coord = glGetAttribLocation(program, attribute_name);
  if (attribute_v_coord == -1) {
    fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
    return 0;
  }
  attribute_name = "v_normal";
  attribute_v_normal = glGetAttribLocation(program, attribute_name);
  if (attribute_v_normal == -1) {
    fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
    return 0;
  }
  const char* uniform_name;
  uniform_name = "m";
  uniform_m = glGetUniformLocation(program, uniform_name);
  if (uniform_m == -1) {
    fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
    return 0;
  }
  uniform_name = "v";
  uniform_v = glGetUniformLocation(program, uniform_name);
  if (uniform_v == -1) {
    fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
    return 0;
  }
  uniform_name = "p";
  uniform_p = glGetUniformLocation(program, uniform_name);
  if (uniform_p == -1) {
    fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
    return 0;
  }
  uniform_name = "m_3x3_inv_transp";
  uniform_m_3x3_inv_transp = glGetUniformLocation(program, uniform_name);
  if (uniform_m_3x3_inv_transp == -1) {
    fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
    return 0;
  }
  uniform_name = "v_inv";
  uniform_v_inv = glGetUniformLocation(program, uniform_name);
  if (uniform_v_inv == -1) {
    fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
    return 0;
  }

  fps_start = glutGet(GLUT_ELAPSED_TIME);

  return 1;
}

void init_view() {
  main_object.object2world = glm::translate(glm::mat4(1), glm::vec3(-2, 1, 0))
    * glm::rotate(glm::mat4(1), 90.0f, glm::vec3(0, 1, 0));
  transforms[MODE_CAMERA] = glm::lookAt(
    glm::vec3(0.0,  1.0, 6.0),   // eye
    glm::vec3(0.0,  1.0, 0.0),   // direction
    glm::vec3(0.0,  1.0, 0.0));  // up

  // transforms[MODE_CAMERA] = glm::lookAt(
  //   glm::vec3( 0.0,  1.0, -1.8),   // eye
  //   glm::vec3(-3.0,  1.0,  0.0),   // direction
  //   glm::vec3( 0.0,  1.0,  0.0));  // up
}

void onSpecial(int key, int x, int y) {
  int modifiers = glutGetModifiers();
  if ((modifiers & GLUT_ACTIVE_ALT) == GLUT_ACTIVE_ALT)
    strife = 1;
  else
    strife = 0;

  if ((modifiers & GLUT_ACTIVE_SHIFT) == GLUT_ACTIVE_SHIFT)
    speed_factor = 0.1;
  else
    speed_factor = 1;

  switch (key) {
  case GLUT_KEY_F1:
    view_mode = MODE_OBJECT;
    break;
  case GLUT_KEY_F2:
    view_mode = MODE_CAMERA;
    break;
  case GLUT_KEY_F3:
    view_mode = MODE_LIGHT;
    break;
  case GLUT_KEY_LEFT:
    rotY_direction = 1;
    break;
  case GLUT_KEY_RIGHT:
    rotY_direction = -1;
    break;
  case GLUT_KEY_UP:
    transZ_direction = 1;
    break;
  case GLUT_KEY_DOWN:
    transZ_direction = -1;
    break;
  case GLUT_KEY_PAGE_UP:
    rotX_direction = -1;
    break;
  case GLUT_KEY_PAGE_DOWN:
    rotX_direction = 1;
    break;
  case GLUT_KEY_HOME:
    init_view();
    break;
  }
}

void onSpecialUp(int key, int x, int y) {
  switch (key) {
  case GLUT_KEY_LEFT:
  case GLUT_KEY_RIGHT:
    rotY_direction = 0;
    break;
  case GLUT_KEY_UP:
  case GLUT_KEY_DOWN:
    transZ_direction = 0;
    break;
  case GLUT_KEY_PAGE_UP:
  case GLUT_KEY_PAGE_DOWN:
    rotX_direction = 0;
    break;
  }
}

/**
 * Get a normalized vector from the center of the virtual ball O to a
 * point P on the virtual ball surface, such that P is aligned on
 * screen's (X,Y) coordinates.  If (X,Y) is too far away from the
 * sphere, return the nearest point on the virtual ball surface.
 */
glm::vec3 get_arcball_vector(int x, int y) {
  glm::vec3 P = glm::vec3(1.0*x/screen_width*2 - 1.0,
			  1.0*y/screen_height*2 - 1.0,
			  0);
  P.y = -P.y;
  float OP_squared = P.x * P.x + P.y * P.y;
  if (OP_squared <= 1*1)
    P.z = sqrt(1*1 - OP_squared);  // Pythagore
  else
    P = glm::normalize(P);  // nearest point
  return P;
}

/**
 * Checks whether the line defined by two points la and lb intersects
 * the portal.
 */
int portal_intersection(const glm::vec4 &la, const glm::vec4 &lb, Mesh* portal) {
  if (la != lb) {  // camera moved
    // Check for intersection with each of the portal's 2 front triangles
    for (int i = 0; i < 2; i++) {
      // Portal coordinates in world view
      glm::vec4
	p0 = portal->object2world * portal->vertices[portal->elements[i*3+0]],
	p1 = portal->object2world * portal->vertices[portal->elements[i*3+1]],
	p2 = portal->object2world * portal->vertices[portal->elements[i*3+2]];

      // Solve line-plane intersection using parametric form
      glm::vec3 tuv =
	glm::inverse(glm::mat3(glm::vec3(la.x - lb.x, la.y - lb.y, la.z - lb.z),
			       glm::vec3(p1.x - p0.x, p1.y - p0.y, p1.z - p0.z),
			       glm::vec3(p2.x - p0.x, p2.y - p0.y, p2.z - p0.z)))
	* glm::vec3(la.x - p0.x, la.y - p0.y, la.z - p0.z);
      float t = tuv.x, u = tuv.y, v = tuv.z;

      // intersection with the plane
      if (t >= 0-1e-6 && t <= 1+1e-6) {
	// intersection with the triangle
	if (u >= 0-1e-6 && u <= 1+1e-6 && v >= 0-1e-6 && v <= 1+1e-6 && (u + v) <= 1+1e-6) {
	  return 1;
	}
      }
    }
  }
  return 0;
}

/**
 * Compute a world2camera view matrix to see from portal 'dst', given
 * the original view and the 'src' portal position.
 */
glm::mat4 portal_view(const glm::mat4 &orig_view, Mesh* src, Mesh* dst) {
  glm::mat4 mv = orig_view * src->object2world;
  glm::mat4 portal_cam =
    // 3. transformation from source portal to the camera - it's the
    //    first portal's ModelView matrix:
    mv
    // 2. object is front-facing, the camera is facing the other way:
    * glm::rotate(glm::mat4(1.0), 180.0f, glm::vec3(0.0,1.0,0.0))
    // 1. go the destination portal; using inverse, because camera
    //    transformations are reversed compared to object
    //    transformations:
    * glm::inverse(dst->object2world)
    ;
  return portal_cam;
}

void logic() {
  /* FPS count */
  {
    fps_frames++;
    int delta_t = glutGet(GLUT_ELAPSED_TIME) - fps_start;
    if (delta_t > 1000) {
      cout << 1000.0 * fps_frames / delta_t << endl;
      fps_frames = 0;
      fps_start = glutGet(GLUT_ELAPSED_TIME);
    }
  }

  glm::mat4 prev_cam = transforms[MODE_CAMERA];

  /* Handle keyboard-based transformations */
  int delta_t = glutGet(GLUT_ELAPSED_TIME) - last_ticks;
  last_ticks = glutGet(GLUT_ELAPSED_TIME);

  float delta_transZ = transZ_direction * delta_t / 1000.0 * 5 * speed_factor;  // 5 units per second
  float delta_transX = 0, delta_transY = 0, delta_rotY = 0, delta_rotX = 0;
  if (strife) {
    delta_transX = rotY_direction * delta_t / 1000.0 * 3 * speed_factor;  // 3 units per second
    delta_transY = rotX_direction * delta_t / 1000.0 * 3 * speed_factor;  // 3 units per second
  } else {
    delta_rotY =  rotY_direction * delta_t / 1000.0 * 120 * speed_factor;  // 120° per second
    delta_rotX = -rotX_direction * delta_t / 1000.0 * 120 * speed_factor;  // 120° per second
  }

  if (view_mode == MODE_OBJECT) {
    main_object.object2world = glm::rotate(main_object.object2world, delta_rotY, glm::vec3(0.0, 1.0, 0.0));
    main_object.object2world = glm::rotate(main_object.object2world, delta_rotX, glm::vec3(1.0, 0.0, 0.0));
    main_object.object2world = glm::translate(main_object.object2world, glm::vec3(0.0, 0.0, delta_transZ));
  } else if (view_mode == MODE_CAMERA) {
    // Camera is reverse-facing, so reverse Z translation and X rotation.
    // Plus, the View matrix is the inverse of the camera2world (it's
    // world->camera), so we'll reverse the transformations.
    // Alternatively, imagine that you transform the world, instead of positioning the camera.
    if (strife) {
      transforms[MODE_CAMERA] = glm::translate(glm::mat4(1.0), glm::vec3(delta_transX, 0.0, 0.0)) * transforms[MODE_CAMERA];
    } else {
      glm::vec3 y_axis_world = glm::mat3(transforms[MODE_CAMERA]) * glm::vec3(0.0, 1.0, 0.0);
      transforms[MODE_CAMERA] = glm::rotate(glm::mat4(1.0), -delta_rotY, y_axis_world) * transforms[MODE_CAMERA];
    }

    if (strife)
      transforms[MODE_CAMERA] = glm::translate(glm::mat4(1.0), glm::vec3(0.0, delta_transY, 0.0)) * transforms[MODE_CAMERA];
    else
      transforms[MODE_CAMERA] = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, delta_transZ)) * transforms[MODE_CAMERA];

    transforms[MODE_CAMERA] = glm::rotate(glm::mat4(1.0), delta_rotX, glm::vec3(1.0, 0.0, 0.0)) * transforms[MODE_CAMERA];
  }

  /* Handle portals */
  // Movement of the camera in world view
  for (int i = 0; i < 2; i++) {
    glm::vec4 la = glm::inverse(prev_cam) * glm::vec4(0.0, 0.0, 0.0, 1.0);
    glm::vec4 lb = glm::inverse(transforms[MODE_CAMERA]) * glm::vec4(0.0, 0.0, 0.0, 1.0);
    if (portal_intersection(la, lb, &portals[i]))
      transforms[MODE_CAMERA] = portal_view(transforms[MODE_CAMERA], &portals[i], &portals[(i+1)%2]);
  }

  /* Handle arcball */
  if (cur_mx != last_mx || cur_my != last_my) {
    glm::vec3 va = get_arcball_vector(last_mx, last_my);
    glm::vec3 vb = get_arcball_vector( cur_mx,  cur_my);
    float angle = acos(min(1.0f, glm::dot(va, vb)));
    glm::vec3 axis_in_camera_coord = glm::cross(va, vb);
    glm::mat3 camera2object = glm::inverse(glm::mat3(transforms[MODE_CAMERA]) * glm::mat3(main_object.object2world));
    glm::vec3 axis_in_object_coord = camera2object * axis_in_camera_coord;
    main_object.object2world = glm::rotate(main_object.object2world, glm::degrees(angle), axis_in_object_coord);
    last_mx = cur_mx;
    last_my = cur_my;
  }

  // Model
  // Set in onDisplay() - cf. main_object.object2world

  // View
  glm::mat4 world2camera = transforms[MODE_CAMERA];

  // Projection
  glm::mat4 camera2screen = glm::perspective(fovy, 1.0f*screen_width/screen_height, zNear, 100.0f);

  glUseProgram(program);
  glUniformMatrix4fv(uniform_v, 1, GL_FALSE, glm::value_ptr(world2camera));
  glUniformMatrix4fv(uniform_p, 1, GL_FALSE, glm::value_ptr(camera2screen));

  glm::mat4 v_inv = glm::inverse(world2camera);
  glUniformMatrix4fv(uniform_v_inv, 1, GL_FALSE, glm::value_ptr(v_inv));

  glutPostRedisplay();
}

/**
 * Draw a frame around the portal.
 */
void draw_portal_bbox(Mesh* portal) {
  // 0.05 frame around the portal
  Mesh portal_bbox;
  portal_bbox.vertices.push_back(glm::vec4(-1.00, -1.05, 0, 1));
  portal_bbox.vertices.push_back(glm::vec4(-1.00,  1.05, 0, 1));
  portal_bbox.vertices.push_back(glm::vec4(-1.05, -1.05, 0, 1));

  portal_bbox.vertices.push_back(glm::vec4(-1.05, -1.05, 0, 1));
  portal_bbox.vertices.push_back(glm::vec4(-1.00,  1.05, 0, 1));
  portal_bbox.vertices.push_back(glm::vec4(-1.05,  1.05, 0, 1));

  portal_bbox.vertices.push_back(glm::vec4( 1.05, -1.05, 0, 1));
  portal_bbox.vertices.push_back(glm::vec4( 1.05,  1.05, 0, 1));
  portal_bbox.vertices.push_back(glm::vec4( 1.00, -1.05, 0, 1));

  portal_bbox.vertices.push_back(glm::vec4( 1.00, -1.05, 0, 1));
  portal_bbox.vertices.push_back(glm::vec4( 1.05,  1.05, 0, 1));
  portal_bbox.vertices.push_back(glm::vec4( 1.00,  1.05, 0, 1));

  portal_bbox.vertices.push_back(glm::vec4(-1.00,  1.05, 0, 1));
  portal_bbox.vertices.push_back(glm::vec4(-1.00,  1.00, 0, 1));
  portal_bbox.vertices.push_back(glm::vec4( 1.00,  1.05, 0, 1));

  portal_bbox.vertices.push_back(glm::vec4( 1.00,  1.05, 0, 1));
  portal_bbox.vertices.push_back(glm::vec4(-1.00,  1.00, 0, 1));
  portal_bbox.vertices.push_back(glm::vec4( 1.00,  1.00, 0, 1));

  portal_bbox.vertices.push_back(glm::vec4(-1.00, -1.00, 0, 1));
  portal_bbox.vertices.push_back(glm::vec4(-1.00, -1.05, 0, 1));
  portal_bbox.vertices.push_back(glm::vec4( 1.00, -1.00, 0, 1));

  portal_bbox.vertices.push_back(glm::vec4( 1.00, -1.00, 0, 1));
  portal_bbox.vertices.push_back(glm::vec4(-1.00, -1.05, 0, 1));
  portal_bbox.vertices.push_back(glm::vec4( 1.00, -1.05, 0, 1));

  for (int i = 0; i < portal_bbox.vertices.size(); i++)
    portal_bbox.normals.push_back(glm::vec3(0,0,1));

  portal_bbox.upload();

  /* Apply object's transformation matrix */
  portal_bbox.object2world = portal->object2world;
  portal_bbox.draw();
  portal_bbox.object2world = portal->object2world * 
    glm::rotate(glm::mat4(1), 180.0f, glm::vec3(0, 1, 0));
  portal_bbox.draw();
}

/**
 * Fill screen with a black square aligned with the perspective
 */
void fill_screen() {
  GLfloat vertices[] = {
    -1, -1, 0, 1,
     1, -1, 0, 1,
    -1,  1, 0, 1,
    -1,  1, 0, 1,
     1, -1, 0, 1,
     1,  1, 0, 1,
  };
  GLuint vbo_vertices;
  glGenBuffers(1, &vbo_vertices);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  /* Apply object's transformation matrix */
  glm::mat4 m = glm::inverse(glm::perspective(fovy, 1.0f*screen_width/screen_height, zNear, 100.0f));
  glUniformMatrix4fv(uniform_m, 1, GL_FALSE, glm::value_ptr(m));

  // Save current view matrix - useful since we're going to
  // recursively draw the scene from different points of View
  GLuint cur_program;
  GLfloat save_v[16];
  GLfloat save_v_inv[16];
  glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&cur_program);
  glGetUniformfv(cur_program, uniform_v, save_v);
  glGetUniformfv(cur_program, uniform_v_inv, save_v_inv);

  glUniformMatrix4fv(uniform_v, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0)));

  glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
  glEnableVertexAttribArray(attribute_v_coord);
  glVertexAttribPointer(
    attribute_v_coord,  // attribute
    4,                  // number of elements per vertex, here (x,y,z,w)
    GL_FLOAT,           // the type of each element
    GL_FALSE,           // take our values as-is
    0,                  // no extra data between each position
    0                   // offset of first element
  );

  glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices)/sizeof(vertices[0])/4);

  glDisableVertexAttribArray(attribute_v_coord);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDeleteBuffers(1, &vbo_vertices);

  // Restore view matrix
  glUniformMatrix4fv(uniform_v, 1, GL_FALSE, save_v);
  glUniformMatrix4fv(uniform_v_inv, 1, GL_FALSE, save_v_inv);
}

void draw_camera() {
  GLfloat vertices[] = {
    -1, -1, 0, 1,
     1, -1, 0, 1,
     1, -1, 0, 1,
    -1,  1, 0, 1,
    -1,  1, 0, 1,
    -1, -1, 0, 1,

    -1,  1, 0, 1,
     1, -1, 0, 1,
     1, -1, 0, 1,
     1,  1, 0, 1,
     1,  1, 0, 1,
    -1,  1, 0, 1,
  };
  GLuint vbo_vertices;
  glGenBuffers(1, &vbo_vertices);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  /* Apply object's transformation matrix */
  glm::mat4 m = glm::inverse(transforms[MODE_CAMERA]);
  glUniformMatrix4fv(uniform_m, 1, GL_FALSE, glm::value_ptr(m));

  glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
  glEnableVertexAttribArray(attribute_v_coord);
  glVertexAttribPointer(
    attribute_v_coord,  // attribute
    4,                  // number of elements per vertex, here (x,y,z,w)
    GL_FLOAT,           // the type of each element
    GL_FALSE,           // take our values as-is
    0,                  // no extra data between each position
    0                   // offset of first element
  );

  glDrawArrays(GL_LINES, 0, 6);
  glDrawArrays(GL_LINES, 6, 6);

  glDisableVertexAttribArray(attribute_v_coord);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDeleteBuffers(1, &vbo_vertices);
}

void draw_portal_stencil(vector<glm::mat4> view_stack, Mesh* portal) {
  GLboolean save_color_mask[4];
  GLboolean save_depth_mask;
  glGetBooleanv(GL_COLOR_WRITEMASK, save_color_mask);
  glGetBooleanv(GL_DEPTH_WRITEMASK, &save_depth_mask);

  //bool debug = (view_stack.size() == 4);
  bool debug = false;

  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  glDepthMask(GL_FALSE);
  glStencilFunc(GL_NEVER, 0, 0xFF);
  glStencilOp(GL_INCR, GL_KEEP, GL_KEEP);  // draw 1s on test fail (always)
  // Note: in Mesa 8's software renderer, nothing is drawn on the
  // stencil buffer, looks like a bug; doesn't happen in stencil/cube.cpp.
  // draw stencil pattern
  glClear(GL_STENCIL_BUFFER_BIT);  // needs mask=0xFF
  glUniformMatrix4fv(uniform_v, 1, GL_FALSE, glm::value_ptr(view_stack[0]));
  portal->draw();
  if (debug) {
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      glClear(GL_COLOR_BUFFER_BIT);
      glStencilMask(0x00);
      glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
      fill_screen();
      glutSwapBuffers();
      cout << "swap" << endl;
      sleep(1);
      glStencilMask(0xFF);
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    }
  for (unsigned int i = 1; i < view_stack.size() - 1; i++) {  // -1 to ignore last view
    // Increment intersection for current portal
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilOp(GL_INCR, GL_KEEP, GL_KEEP);  // draw 1s on test fail (always)
    glUniformMatrix4fv(uniform_v, 1, GL_FALSE, glm::value_ptr(view_stack[i]));
    portal->draw();
    if (debug) {
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      glClear(GL_COLOR_BUFFER_BIT);
      glStencilMask(0x00);
      glStencilFunc(GL_LEQUAL, 1, 0xFF);
      glDisable(GL_STENCIL_TEST);
      portal->draw();
      glEnable(GL_STENCIL_TEST);
      //fill_screen();
      glutSwapBuffers();
      cout << "swap" << endl;
      sleep(1);
      glStencilMask(0xFF);
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    }
    // Decremental outer portal -> only sub-portal intersection remains
    glStencilFunc(GL_NEVER, 0, 0xFF);
    glStencilOp(GL_DECR, GL_KEEP, GL_KEEP);  // draw 1s on test fail (always)
    glUniformMatrix4fv(uniform_v, 1, GL_FALSE, glm::value_ptr(view_stack[i-1]));
    portal->draw();
    if (debug) {
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      glClear(GL_COLOR_BUFFER_BIT);
      glStencilMask(0x00);
      glStencilFunc(GL_LEQUAL, 1, 0xFF);
      fill_screen();
      glutSwapBuffers();
      cout << "swap" << endl;
      sleep(1);
      glStencilMask(0xFF);
      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    }
  }

  //glColorMask(GL_TRUE, GL_TRUE, GL_FALSE, GL_TRUE);  // blue-ish filter if drawing on white or grey
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDepthMask(GL_TRUE);
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
  /* Fill 1 or more */
  glStencilFunc(GL_LEQUAL, 1, 0xFF);
  // glutSwapBuffers();
  // cout << "rec=" << rec << ", i=" << i << endl;
  // sleep(1);
  glColorMask(save_color_mask[0], save_color_mask[1], save_color_mask[2], save_color_mask[3]);
  glDepthMask(save_depth_mask);
  glUniformMatrix4fv(uniform_v, 1, GL_FALSE, glm::value_ptr(view_stack.back()));
  // -Ready to draw main scene-
}

bool clip_portal(vector<glm::mat4> view_stack, Mesh* outer_portal, rect* scissor) {
  scissor->x = 0;
  scissor->y = 0;
  scissor->w = screen_width;
  scissor->h = screen_height;
  for (unsigned int v = 0; v < view_stack.size() - 1; v++) {  // -1 to ignore last view
    glm::vec4 p[4];
    rect r;
    bool found_negative_w = false;
    for (int pi = 0; pi < 4; pi++) {
      p[pi] = (glm::perspective(fovy, 1.0f*screen_width/screen_height, zNear, 100.0f)
	      * view_stack[v]
	      * outer_portal->object2world)
	* outer_portal->vertices[pi];
      if (p[pi].w < 0) {
	// TODO: I tried to deal with that case, but it's quite
	// complex because this means the coordinate is projected from
	// the back of the camera, and we should clip to the min or
	// max of the screen.  I'll let the stencil buffer deal with
	// it for now.
	// Possible fix: restrict the portal rectangle using its line
	// intersection with the camera frustum.
	//cout << "w<0" << endl;
	//glDisable(GL_SCISSOR_TEST);
	found_negative_w = true;
      } else {
	p[pi].x /= p[pi].w;
	p[pi].y /= p[pi].w;
      }
    }
    if (found_negative_w)
      continue;

    glm::vec4 min_x, max_x, max_y, min_y;
    min_x = max_x = min_y = max_y = p[0];
    for (int i = 0; i < 4; i++) {
      if (p[i].x < min_x.x) min_x = p[i];
      if (p[i].x > max_x.x) max_x = p[i];
      if (p[i].y < min_y.y) min_y = p[i];
      if (p[i].y > max_y.y) max_y = p[i];
    }

    // (broken) attempt to deal with w < 0
    // if (min_x.w < 0) { min_x.x = max_x.x; max_x.x =  1; }
    // if (max_x.w < 0) { max_x.x = min_x.x; min_x.x = -1; }

    min_x.x = (max(-1.0f, min_x.x) + 1) / 2 * screen_width;
    max_x.x = (min( 1.0f, max_x.x) + 1) / 2 * screen_width;
    min_y.y = (max(-1.0f, min_y.y) + 1) / 2 * screen_height;
    max_y.y = (min( 1.0f, max_y.y) + 1) / 2 * screen_height;

    r.x = min_x.x;
    r.y = min_y.y;
    r.w = max_x.x-min_x.x;
    r.h = max_y.y-min_y.y;

    // intersection with previous rect
    //cout << "+" << scissor->x << "," << scissor->y << "," << scissor->w << "," << scissor->h << endl;
    //cout << "+" << r.x << "," << r.y << "," << r.w << "," << r.h << endl;
    {
      int r_min_x = max(r.x, scissor->x);
      int r_max_x = min(r.x+r.w, scissor->x+scissor->w);
      scissor->x = r_min_x;
      scissor->w = r_max_x - scissor->x;
      int r_min_y = max(r.y, scissor->y);
      int r_max_y = min(r.y+r.h, scissor->y+scissor->h);
      scissor->y = r_min_y;
      scissor->h = r_max_y - scissor->y;
    }
    //cout << "=" << scissor->x << "," << scissor->y << "," << scissor->w << "," << scissor->h << endl;
    if (scissor->w <= 0 || scissor->h <= 0) {
      return false;
    }
  }
  
  //cout << scissor->x << "," << scissor->y << "," << scissor->w << "," << scissor->h << endl;
  //cout << endl;
  return true;
}

/**
 * Draw the active portals contents
 */
void draw_portals(vector<glm::mat4> view_stack, int rec, int outer_portal) {
  //if (rec >= 2) return;
  // TODO: replace rec with size threshold for MV * portal.bbox ?

  GLboolean save_stencil_test;
  glGetBooleanv(GL_STENCIL_TEST, &save_stencil_test);

  glEnable(GL_STENCIL_TEST);
  glEnable(GL_SCISSOR_TEST);
  for (int i = 0; i < 2; i++) {
    // Important: don't draw outer_portal's outgoing portal - only
    // draw the same portal when displaying a sub-portal (seen from
    // the other portal).
    if (outer_portal == -1 || i == outer_portal) {
      glm::mat4 portal_cam = portal_view(view_stack.back(), &portals[i], &portals[(i+1)%2]);
      view_stack.push_back(portal_cam);
      // draw_portal_stencil(view_stack, &portals[i]);
      draw_scene(view_stack, rec + 1, i);
      view_stack.pop_back();
      glUniformMatrix4fv(uniform_v, 1, GL_FALSE, glm::value_ptr(view_stack.back()));
      glUniformMatrix4fv(uniform_v_inv, 1, GL_FALSE, glm::value_ptr(glm::inverse(view_stack.back())));
      // TODO: write something without lines, I don't have confidence in its interaction with the stencil buffer
      //glLineWidth(1);
    }
  }
  if (!save_stencil_test) {
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_SCISSOR_TEST);
  }

  // Draw portal in the depth buffer so they are not overwritten
  glClear(GL_DEPTH_BUFFER_BIT);

  GLboolean save_color_mask[4];
  GLboolean save_depth_mask;
  glGetBooleanv(GL_COLOR_WRITEMASK, save_color_mask);
  glGetBooleanv(GL_DEPTH_WRITEMASK, &save_depth_mask);
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  glDepthMask(GL_TRUE);
  for (int i = 0; i < 2; i++)
    portals[i].draw();
  glColorMask(save_color_mask[0], save_color_mask[1], save_color_mask[2], save_color_mask[3]);
  glDepthMask(save_depth_mask);
}

void draw_scene(vector<glm::mat4> view_stack, int rec, int outer_portal = -1) {
  if (rec >= 5) {
    //draw_portal_stencil(view_stack, &portals[outer_portal]);
    //draw_mesh(&portals[(outer_portal+1)%2]);
    return;
  }
  rect scissor;
  if (outer_portal != -1) {
    // if basic clipping returns an empty rectangle, we can stop here
    if (!clip_portal(view_stack, &portals[outer_portal], &scissor))
      return;
  }

  // Set view matrix
  glUniformMatrix4fv(uniform_v, 1, GL_FALSE, glm::value_ptr(view_stack.back()));
  glUniformMatrix4fv(uniform_v_inv, 1, GL_FALSE, glm::value_ptr(glm::inverse(view_stack.back())));

  glClear(GL_DEPTH_BUFFER_BIT);

  // Draw portals contents
  draw_portals(view_stack, rec, outer_portal);

  if (outer_portal != -1) {
    // clip the current view as much as possible, more efficient than
    // using the stencil buffer
    glScissor(scissor.x, scissor.y, scissor.w, scissor.h);

    // draw the current stencil - or actually recreate it if we just
    // drew a sub-portal and hence messed the stencil buffer
    draw_portal_stencil(view_stack, &portals[outer_portal]);
  }
  
  // Draw portals frames after the stencil buffer is set
  for (int i = 0; i < 2; i++) {
    draw_portal_bbox(&portals[i]);
    //portals[i].draw_bbox();
  }
  
  /* Draw scene */
  //light_bbox.draw_bbox();
  main_object.draw();
  ground.draw();
  //portals[0].draw();

  // Restore view matrix
  view_stack.pop_back();
  if (view_stack.size() > 0) {
    glUniformMatrix4fv(uniform_v, 1, GL_FALSE, glm::value_ptr(view_stack.back()));
    glUniformMatrix4fv(uniform_v_inv, 1, GL_FALSE, glm::value_ptr(glm::inverse(view_stack.back())));
  }

  // glutSwapBuffers();
  // cout << "rec=" << rec << endl;
  // sleep(2);
}

void draw() {
  glClearColor(0.45, 0.45, 0.45, 1.0);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

  glUseProgram(program);

  vector<glm::mat4> view_stack;
  view_stack.push_back(transforms[MODE_CAMERA]);

  glViewport(0, 0, screen_width, screen_height);
  draw_scene(view_stack, 1);

  glViewport(2*screen_width/3, 0, screen_width/3, screen_height/3);
  glClear(GL_DEPTH_BUFFER_BIT);
  view_stack.clear();
  view_stack.push_back(glm::lookAt(
    glm::vec3(0.0,  9.0,-2.0),   // eye
    glm::vec3(0.0,  0.0,-2.0),   // direction
    glm::vec3(0.0,  0.0,-1.0))   // up
  );
  draw_scene(view_stack, 4);
  draw_camera();
}

void onDisplay()
{
  logic();
  draw();
  glutSwapBuffers();
}

void onMouse(int button, int state, int x, int y) {
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    arcball_on = true;
    last_mx = cur_mx = x;
    last_my = cur_my = y;
  } else {
    arcball_on = false;
  }
}

void onMotion(int x, int y) {
  if (arcball_on) {  // if left button is pressed
    cur_mx = x;
    cur_my = y;
  }
}

void onReshape(int width, int height) {
  screen_width = width;
  screen_height = height;
  glViewport(0, 0, screen_width, screen_height);
  create_portal(&portals[0], screen_width, screen_height, zNear, fovy);
  create_portal(&portals[1], screen_width, screen_height, zNear, fovy);
}

void free_resources()
{
  glDeleteProgram(program);
}


int main(int argc, char* argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA|GLUT_ALPHA|GLUT_DOUBLE|GLUT_DEPTH|GLUT_STENCIL);
  glutInitWindowSize(screen_width, screen_height);
  glutCreateWindow("Mini-Portal");

  GLenum glew_status = glewInit();
  if (glew_status != GLEW_OK) {
    fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
    return 1;
  }

  if (!GLEW_VERSION_2_0) {
    fprintf(stderr, "Error: your graphic card does not support OpenGL 2.0\n");
    return 1;
  }

  char* obj_filename = (char*) "cube.obj";
  char* v_shader_filename = (char*) "phong-shading.v.glsl";
  char* f_shader_filename = (char*) "phong-shading.f.glsl";
  if (argc != 4) {
    fprintf(stderr, "Usage: %s model.obj vertex_shader.v.glsl fragment_shader.f.glsl\n", argv[0]);
  } else {
    obj_filename = argv[1];
    v_shader_filename = argv[2];
    f_shader_filename = argv[3];
  }

  if (init_resources(obj_filename, v_shader_filename, f_shader_filename)) {
    init_view();
    glutDisplayFunc(onDisplay);
    glutSpecialFunc(onSpecial);
    glutSpecialUpFunc(onSpecialUp);
    glutMouseFunc(onMouse);
    glutMotionFunc(onMotion);
    glutReshapeFunc(onReshape);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    // Make bounding box clearer against the ground:
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1, 0);
    last_ticks = glutGet(GLUT_ELAPSED_TIME);
    glutMainLoop();
  }

  free_resources();
  return 0;
}

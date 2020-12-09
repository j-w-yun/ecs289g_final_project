#pragma once

#include <utility>
#include <fstream>

#include "Stat.h"
#include "Stat.cpp"
#include "Input.h"
#include "Input.cpp"
#include "Map.h"
#include "Map.cpp"
#include "Stat.h"
#include "Stat.cpp"
#include "Vector2f.h"
#include "Vector2f.cpp"
#include "World.h"
#include "World.cpp"

struct Dimension {
	int top;
	int right;
	int bottom;
	int left;
};

Dimension get_dimension(const std::vector<Vector2f>& vs) {
	// Find range of x and y
	Dimension d;
	d.left = vs[0].x();
	d.right = vs[0].x();
	d.top = vs[0].y();
	d.bottom = vs[0].y();
	for (int j = 0; j < (int)vs.size(); j++) {
		d.left = vs[j].x() < d.left ? vs[j].x() : d.left;
		d.right = vs[j].x() > d.right ? vs[j].x() : d.right;
		d.top = vs[j].y() < d.top ? vs[j].y() : d.top;
		d.bottom = vs[j].y() > d.bottom ? vs[j].y() : d.bottom;
	}
	return d;
}

double cubic_interpolate(const double y0, const double y1, const double y2, const double y3, const double mu) {
	double mu2 = mu * mu;
	double a0 = y3 - y2 - y0 + y1;
	double a1 = y0 - y1 - a0;
	double a2 = y2 - y0;
	double a3 = y1;
	return a0*mu*mu2 + a1*mu2 + a2*mu + a3;
}

std::vector<Vector2f> cubic_interpolate(const std::vector<Vector2f>& vs, const int np) {
	int nv = (int)vs.size();
	std::vector<Vector2f> ps;
	ps.resize(np*nv);
	for (int j = 0; j < nv; j++) {
		Vector2f v1 = vs[j];
		Vector2f v2 = vs[(j+1)%nv];
		Vector2f v3 = vs[(j+2)%nv];
		Vector2f v4 = vs[(j+3)%nv];
		for (int k = 0; k < np; k++) {
			ps[j*np+k].setx(cubic_interpolate(v1.x(), v2.x(), v3.x(), v4.x(), (double)k/np));
			ps[j*np+k].sety(cubic_interpolate(v1.y(), v2.y(), v3.y(), v4.y(), (double)k/np));
		}
	}
	return ps;
}

double hermite_interpolate(const double y0, const double y1, const double y2, const double y3, const double mu, const double tension, const double bias) {
	double mu2 = mu * mu;
	double mu3 = mu2 * mu;
	double m0 = (y1-y0) * (1+bias) * (1-tension)/2 + (y2-y1) * (1-bias) * (1-tension)/2;
	double m1 = (y2-y1) * (1+bias) * (1-tension)/2 + (y3-y2) * (1-bias) * (1-tension)/2;
	double a0 = 2 * mu3 - 3 * mu2 + 1;
	double a1 = mu3 - 2 * mu2 + mu;
	double a2 = mu3 - mu2;
	double a3 = -2 * mu3 + 3 * mu2;
	return a0*y1 + a1*m0 + a2*m1 + a3*y2;
}

std::vector<Vector2f> hermite_interpolate(const std::vector<Vector2f>& vs, const int np, const double tension, const double bias) {
	int nv = (int)vs.size();
	std::vector<Vector2f> ps;
	ps.resize(np*nv);
	for (int j = 0; j < nv; j++) {
		Vector2f v1 = vs[j];
		Vector2f v2 = vs[(j+1)%nv];
		Vector2f v3 = vs[(j+2)%nv];
		Vector2f v4 = vs[(j+3)%nv];
		for (int k = 0; k < np; k++) {
			ps[j*np+k].setx(hermite_interpolate(v1.x(), v2.x(), v3.x(), v4.x(), (double)k/np, tension, bias));
			ps[j*np+k].sety(hermite_interpolate(v1.y(), v2.y(), v3.y(), v4.y(), (double)k/np, tension, bias));
		}
	}
	return ps;
}

Vector2f bspline_interpolate(const Vector2f& p0, const Vector2f& p1, const Vector2f& p2, const Vector2f& p3, const double mu) {
	double mu2 = mu * mu;
	double mu3 = mu2 * mu;
	double mt = 1 - mu;
	double mt3 = mt * mt * mt;
	double bi0 = mt3;
	double bi1 = 3 * mu3 - 6 * mu2 + 4;
	double bi2 = -3 * mu3 + 3 * mu2 + 3 * mu + 1;
	double bi3 = mu3;
	return Vector2f(
		(p0.x()*bi0 + p1.x()*bi1 +p2.x()*bi2 + p3.x()*bi3)/6,
		(p0.y()*bi0 + p1.y()*bi1 +p2.y()*bi2 + p3.y()*bi3)/6
	);
}

std::vector<Vector2f> bspline_interpolate(const std::vector<Vector2f>& vs, const int np) {
	int nv = (int)vs.size();
	std::vector<Vector2f> ps;
	ps.resize(np*nv);
	for (int j = 0; j < nv; j++) {
		Vector2f v1 = vs[j];
		Vector2f v2 = vs[(j+1)%nv];
		Vector2f v3 = vs[(j+2)%nv];
		Vector2f v4 = vs[(j+3)%nv];
		for (int k = 0; k < np; k++)
			ps[j*np+k] = bspline_interpolate(v1, v2, v3, v4, (double)k/np);
	}
	return ps;
}

std::vector<Vector2f> weighted_average(std::vector<Vector2f> vs1, std::vector<Vector2f> vs2, float w1, float w2) {
	std::vector<Vector2f> ps;
	ps.resize(vs1.size());
	for (int j = 0; j < (int)vs1.size(); j++) {
		ps[j] = (vs1[j]*w1+vs2[j]*w2) / (w1+w2);
	}
	return ps;
}

namespace RenderingEngine {

	// Default window dimensions
	const int DEFAULT_WINDOW_WIDTH = 1200;
	const int DEFAULT_WINDOW_HEIGHT = 800;

	// Font for all rendered text
	const char* TTF_FILE = "./res/fonts/Roboto-Light.ttf";
	const int FONT_SIZE = 16;

	// Distance of mouse from window edge to activate map panning
	const int PAN_REGION_SIZE = 10;

	// Camera settings
	const float PAN_SPEED = 0.3f;
	const float ZOOM_KEY_SPEED = 0.01f;
	const float DEFAULT_ZOOM = 8.0f;
	const float MIN_ZOOM = 1.0f;
	const float MAX_ZOOM = 20.0f;

	/**
	Used to convert world space into screen coordinates.
	*/
	class Camera {
		public:
			Vector2f position = Vector2f(-1, -1);
			float zoom = DEFAULT_ZOOM;
			float get_zoom_factor() {
				return zoom*zoom/100.0f + 0.5f;
			}
	};

	// Player camera
	Camera cam;

	// Track mouse input states
	Vector2f ldrag_start = Vector2f(-1, -1);
	Vector2f rdrag_start = Vector2f(-1, -1);
	bool lbutton_down = false;
	bool rbutton_down = false;

	// Screen dimensions
	int width = DEFAULT_WINDOW_WIDTH;
	int height = DEFAULT_WINDOW_HEIGHT;

	// World dimensions
	float world_width;
	float world_height;

	// SDL subsystems to free / destory later
	SDL_Window* gWindow;
	SDL_Renderer* gRenderer;
	TTF_Font* gFont;
	World gWorld;

#ifndef USE_SDL2_RENDERER
	enum ShaderProgramType {
		Generic2D = 0,
		Generic2D_PerlinNoise,
		Total
	};
	GLuint gShaderProgramIDs[ShaderProgramType::Total] = { 0, 0 };
	ShaderProgramType gCurrentType;

	std::string read_shader_file(const GLchar* filename)
	{
		std::ifstream t(filename);
		t.seekg(0, std::ios::end);
		size_t size = t.tellg();
		std::string buffer(size, ' ');
		t.seekg(0);
		t.read(&buffer[0], size);
		return buffer;
	}

	// OpenGL shaders
	int ogl_prepare_generic_2d_shaders(int shader_type) {
		//Generate program
		gShaderProgramIDs[shader_type] = glCreateProgram();

		//Create vertex shader
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

		//Get vertex source. TODO: We should provide vertex stream during rendering.
		const GLchar* vertexShaderSource[ShaderProgramType::Total];
		vertexShaderSource[Generic2D] = vertexShaderSource[Generic2D_PerlinNoise] = "./res/shaders/generic2d.vs";

		std::string shader_content = read_shader_file(vertexShaderSource[shader_type]);
		const GLchar* shader_content_c_str = shader_content.c_str();

		//Set vertex source
		glShaderSource(vertexShader, 1, &shader_content_c_str, NULL);

		//Compile vertex source
		glCompileShader(vertexShader);

		//Check vertex shader for errors
		GLint vShaderCompiled = GL_FALSE;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
		if (vShaderCompiled != GL_TRUE)
		{
			printf("Unable to compile vertex shader %d!\n", vertexShader);
			return -1;
		}

		//Attach vertex shader to program
		glAttachShader(gShaderProgramIDs[shader_type], vertexShader);

		//Create fragment shader
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		//Get fragment source, TODO: reference a texture object later
		const GLchar* fragmentShaderSource[ShaderProgramType::Total];
		fragmentShaderSource[Generic2D] = "./res/shaders/generic2d.ps";;
		fragmentShaderSource[Generic2D_PerlinNoise] = "./res/shaders/generic2d_perlinnoise.ps";

		shader_content = read_shader_file(fragmentShaderSource[shader_type]);
		shader_content_c_str = shader_content.c_str();

		//Set fragment source
		glShaderSource(fragmentShader, 1, &shader_content_c_str, NULL);

		//Compile fragment source
		glCompileShader(fragmentShader);

		//Check fragment shader for errors
		GLint fShaderCompiled = GL_FALSE;
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
		if (fShaderCompiled != GL_TRUE)
		{
			printf("Unable to compile fragment shader %d!\n", fragmentShader);
			return -2;
		}

		//Attach fragment shader to program
		glAttachShader(gShaderProgramIDs[shader_type], fragmentShader);

		glBindAttribLocation(gShaderProgramIDs[shader_type], 0, "in_Position");
		glBindAttribLocation(gShaderProgramIDs[shader_type], 1, "in_Color");

		//Link program
		glLinkProgram(gShaderProgramIDs[shader_type]);

		//Check for errors
		GLint programSuccess = GL_TRUE;
		glGetProgramiv(gShaderProgramIDs[shader_type], GL_LINK_STATUS, &programSuccess);
		if (programSuccess != GL_TRUE)
		{
			printf("Error linking program %d!\n", gShaderProgramIDs[shader_type]);
			return -3;
		}

		//Additional processing
		if (shader_type == ShaderProgramType::Generic2D_PerlinNoise)
		{
			GLuint blockIndex = glGetUniformBlockIndex(gShaderProgramIDs[shader_type], "in_GlobalBuffer");
			GLint blockSize;

			glGetActiveUniformBlockiv(gShaderProgramIDs[shader_type], blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

			GLubyte* blockBuffer = (GLubyte*)malloc(blockSize);
		}
	}

	void set_shader(int type) {
		gCurrentType = (ShaderProgramType)type;
	}

	GLuint get_current_shader() {
		return gShaderProgramIDs[gCurrentType];
	}

	typedef struct _pure_color_vertex {
		GLfloat location[4];
		GLfloat color[4];
	} pure_color_vertex;

	std::vector<pure_color_vertex> lines;
	GLuint vao_line;
	GLuint vbo_line;
	int total_vertices;

	void ogl_reserve_line_objects(int num) {
		int num_vertices = num * 2;

		glGenVertexArrays(1, &vao_line);
		glBindVertexArray(vao_line);

		lines.resize(num_vertices);

		// Create a Vector Buffer Object that will store the vertices on video memory
		glGenBuffers(1, &vbo_line);

		// Create VBO
		glBindBuffer(GL_ARRAY_BUFFER, vbo_line);
		glBufferData(GL_ARRAY_BUFFER, sizeof(pure_color_vertex) * num_vertices, &lines[0], GL_DYNAMIC_DRAW);

		GLint position_attribute = glGetAttribLocation(gShaderProgramIDs[ShaderProgramType::Generic2D], "in_Position");
		GLint color_attribute = glGetAttribLocation(gShaderProgramIDs[ShaderProgramType::Generic2D], "in_Color");
		glVertexAttribPointer(position_attribute, 4, GL_FLOAT, GL_FALSE, sizeof(pure_color_vertex), (const void*)offsetof(pure_color_vertex, location));
		glVertexAttribPointer(color_attribute, 4, GL_FLOAT, GL_FALSE, sizeof(pure_color_vertex), (const void*)offsetof(pure_color_vertex, color));

		glLineWidth(1.0f);

		total_vertices = 0;
	}

	GLfloat ogl_primitive_color[4];

	void ogl_set_color(GLuint r, GLuint g, GLuint b, GLuint a)
	{
		ogl_primitive_color[0] = r / 255.0f;
		ogl_primitive_color[1] = g / 255.0f;
		ogl_primitive_color[2] = b / 255.0f;
		ogl_primitive_color[3] = a / 255.0f;
	}

	void ogl_send_lines_to_draw() {
		if (total_vertices > 0) {
			glBindVertexArray(vao_line);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_line);
			// get pointer
			void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
			// now copy data into memory
			memcpy(ptr, &lines[0], sizeof(pure_color_vertex) * total_vertices);
			// make sure to tell OpenGL we're done with the pointer
			glUnmapBuffer(GL_ARRAY_BUFFER);

			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glUseProgram(gShaderProgramIDs[0]);

			glEnable(GL_BLEND);

			glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

			glDrawArrays(GL_LINES, 0, total_vertices);

			total_vertices = 0;
		}
	}

	void ogl_draw_line(GLfloat x1, GLfloat x2, GLfloat y1, GLfloat y2) {
		pure_color_vertex v[2];

		// y axis invert
		v[0].location[0] = 2 * (x1 / width - 0.5f);
		v[0].location[1] = 2 * (1.0f - x2 / height - 0.5f);

		memcpy(v[0].color, ogl_primitive_color, sizeof(ogl_primitive_color));

		// y axis invert
		v[1].location[0] = 2 * (y1 / width - 0.5f);
		v[1].location[1] = 2 * (1.0f - y2 / height - 0.5f);

		memcpy(v[1].color, ogl_primitive_color, sizeof(ogl_primitive_color));

		lines[total_vertices] = v[0];
		lines[total_vertices + 1] = v[1];

		total_vertices += 2;

		if (total_vertices >= lines.size() - 2)
			ogl_send_lines_to_draw();
	}

	GLuint vao_rect;
	GLuint vbo_rect;
	std::vector<pure_color_vertex> rects;
	int total_rect_vertices = 0;

	void ogl_reserve_rect_objects(int num) {
		int num_vertices = num * 6;

		glGenVertexArrays(1, &vao_rect);
		glBindVertexArray(vao_rect);

		rects.resize(num_vertices);

		// Create a Vector Buffer Object that will store the vertices on video memory
		glGenBuffers(1, &vbo_rect);

		// Create VBO
		glBindBuffer(GL_ARRAY_BUFFER, vbo_rect);
		glBufferData(GL_ARRAY_BUFFER, sizeof(pure_color_vertex) * num_vertices, &rects[0], GL_DYNAMIC_DRAW);

		GLint position_attribute = glGetAttribLocation(gShaderProgramIDs[ShaderProgramType::Generic2D], "in_Position");
		GLint color_attribute = glGetAttribLocation(gShaderProgramIDs[ShaderProgramType::Generic2D], "in_Color");
		glVertexAttribPointer(position_attribute, 4, GL_FLOAT, GL_FALSE, sizeof(pure_color_vertex), (const void*)offsetof(pure_color_vertex, location));
		glVertexAttribPointer(color_attribute, 4, GL_FLOAT, GL_FALSE, sizeof(pure_color_vertex), (const void*)offsetof(pure_color_vertex, color));

		total_rect_vertices = 0;
	}

	void ogl_send_rects_to_draw() {
		if (total_rect_vertices > 0) {
			glBindVertexArray(vao_rect);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_rect);
			// get pointer
			void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
			// now copy data into memory
			memcpy(ptr, &rects[0], sizeof(pure_color_vertex) * total_rect_vertices);
			// make sure to tell OpenGL we're done with the pointer
			glUnmapBuffer(GL_ARRAY_BUFFER);

			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glUseProgram(gShaderProgramIDs[0]);

			glEnable(GL_BLEND);
			glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

			glDrawArrays(GL_TRIANGLES, 0, total_rect_vertices);

			total_rect_vertices = 0;
		}
	}

	void ogl_fill_rect(SDL_Rect& box) {
		pure_color_vertex v[6];
		GLfloat fWidth = (GLfloat)width;
		GLfloat fHeight = (GLfloat)height;
		v[0].location[0] = 2 * (box.x / fWidth - 0.5f);
		v[0].location[1] = 2 * (1.0f - box.y / fHeight - 0.5f);
		v[1].location[0] = v[0].location[0];
		v[1].location[1] = 2 * (1.0f - (box.y + box.h) / fHeight - 0.5f);
		v[2].location[0] = 2 * ((box.x + box.w) / fWidth - 0.5f);
		v[2].location[1] = v[0].location[1];
		v[3].location[0] = v[2].location[0];
		v[3].location[1] = v[2].location[1];
		v[4].location[0] = v[1].location[0];
		v[4].location[1] = v[1].location[1];
		v[5].location[0] = v[2].location[0];
		v[5].location[1] = v[1].location[1];

		for (int i = 0; i < 6; i++)
		{
			memcpy(v[i].color, ogl_primitive_color, sizeof(ogl_primitive_color));
			rects[total_rect_vertices + i] = v[i];
		}

		total_rect_vertices += 6;

		if (total_rect_vertices >= rects.size())
			ogl_send_rects_to_draw();
	}
#endif

	/**
	Set the world to render.
	*/
	void set_world(const  World& world) {
		gWorld = world;

		if (gWorld.num_levels() == 0)
			return;

		// Get world size
		MapLevel& level = gWorld.get_level(0);
		world_width = level.get_tile_width() * level.get_width();
		world_height = level.get_tile_height() * level.get_height();
	}

	/**
	Convert world position to screen coordinate.
	*/
	Vector2f world_to_screen(Vector2f world_vec) {
		Vector2f screen_vec = world_vec;
		screen_vec = screen_vec.sub(cam.position);
		screen_vec = screen_vec.scale(cam.get_zoom_factor());
		screen_vec = screen_vec.add(width/2, height/2);
		return screen_vec;
	}

	Vector2f world_to_screen(float x, float y) {
		return world_to_screen(Vector2f(x, y));
	}

	/**
	Convert screen coordinate to world position.
	*/
	Vector2f screen_to_world(Vector2f screen_vec) {
		Vector2f world_vec = screen_vec;
		world_vec = world_vec.sub(width/2, height/2);
		world_vec = world_vec.scale(1.0f/cam.get_zoom_factor());
		world_vec = world_vec.add(cam.position);
		return world_vec;
	}

	Vector2f screen_to_world(float x, float y) {
		return screen_to_world(Vector2f(x, y));
	}

	/**
	Fill inside a polygon.
	Set color using SDL_SetRenderDrawColor() beforehand.
	*/
	void fill_poly(std::vector<Vector2f> ps) {
		if (ps.size() == 0)
			return;
		// Find dimensions
		Dimension dim = get_dimension(ps);
		// Sweep line
		std::vector<int> node_x;
		for (int y = dim.top; y < dim.bottom; y++) {
			node_x.resize(ps.size());
			int nodes = 0;
			int j = (int)ps.size() - 1;
			for (int i = 0; i < (int)ps.size(); i++) {
				if ((ps[i].y() < (double)y && ps[j].y() >= (double)y) || (ps[j].y() < (double)y && ps[i].y() >= (double)y))
					node_x[nodes++] = (int)(ps[i].x()+(y-ps[i].y())/(ps[j].y()-ps[i].y())*(ps[j].x()-ps[i].x()));
				j = i;
			}
			// Sort
			node_x.resize(nodes);
			std::sort(node_x.begin(), node_x.end());
			// Draw lines
			for (int i = 0; i < nodes; i += 2) {
				if (node_x[i] >= dim.right)
					break;
				if (node_x[i+1] > dim.left) {
					if (node_x[i] < dim.left)
						node_x[i] = dim.left;
					if (node_x[i+1] > dim.right)
						node_x[i+1] = dim.right;
				// TODO: create polygon to fill. filling lines still feel not as effective.
#ifdef USE_SDL2_RENDERER
					SDL_RenderDrawLine(gRenderer, node_x[i], y, node_x[i+1], y);
#else
					ogl_draw_line(node_x[i], y, node_x[i + 1], y);
#endif
				}
			}
		}
	}

	void clear() {
		// Get current window size
		SDL_GetWindowSize(gWindow, &width, &height);
#ifdef USE_SDL2_RENDERER
		// Clear screen
		SDL_SetRenderDrawColor(gRenderer, 16, 16, 16, 255);
		SDL_RenderClear(gRenderer);
#else
		glClearColor(16.0 / 255.0, 16.0 / 255.0, 16.0 / 255.0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
#endif
	}

	void render(float delta_time) {
		// Move camera to center if unset
		if (cam.position.x() < 0 && cam.position.y() < 0)
			cam.position.set(world_width/2, world_height/2);

		if (Input::has_input()) {
			// Set stats
			std::pair<int, int> mp = Input::get_mouse_pos();
			Vector2f mouse_screen = Vector2f(mp.first, mp.second);
			Vector2f mouse_world = screen_to_world(mouse_screen);
			Stat::mouse_info(mouse_screen, mouse_world);
			Stat::camera_info(cam.position, cam.zoom);

			// Zoom map
			cam.zoom += (float)Input::get_scrolly();
			if (Input::is_key_pressed(SDLK_PAGEUP))
				cam.zoom += delta_time * ZOOM_KEY_SPEED;
			else if (Input::is_key_pressed(SDLK_PAGEDOWN))
				cam.zoom -= delta_time * ZOOM_KEY_SPEED;
			if (cam.zoom < MIN_ZOOM)
				cam.zoom = MIN_ZOOM;
			else if (cam.zoom > MAX_ZOOM)
				cam.zoom = MAX_ZOOM;

			// Pan map
			Vector2f dp = Vector2f(0, 0);
			if (mp.first < PAN_REGION_SIZE || Input::is_key_pressed(SDLK_LEFT))
				dp.setx(-PAN_SPEED);
			if (mp.first > width - PAN_REGION_SIZE || Input::is_key_pressed(SDLK_RIGHT))
				dp.setx(PAN_SPEED);
			if (mp.second < PAN_REGION_SIZE || Input::is_key_pressed(SDLK_UP))
				dp.sety(-PAN_SPEED);
			if (mp.second > height - PAN_REGION_SIZE || Input::is_key_pressed(SDLK_DOWN))
				dp.sety(PAN_SPEED);
			dp = dp.scale(delta_time);

			// Clip camera position
			cam.position = cam.position.add(dp);
			if (cam.position.x() < 0)
				cam.position.setx(0);
			if (cam.position.x() > world_width)
				cam.position.setx(world_width);
			if (cam.position.y() < 0)
				cam.position.sety(0);
			if (cam.position.y() > world_height)
				cam.position.sety(world_height);

			// Send click world coordinates to World
			if (Input::is_mouse_pressed(SDL_BUTTON_LEFT)) {
				if (!lbutton_down) {
					// Button down
					std::pair<int, int> pos = Input::get_mouse_pos();
					Vector2f world_pos = screen_to_world(pos.first, pos.second);
					gWorld.click(world_pos, 0, 0);
					lbutton_down = true;
				}
			}
			else if (lbutton_down) {
				// Button released
				std::pair<int, int> pos = Input::get_mouse_pos();
				Vector2f world_pos = screen_to_world(pos.first, pos.second);
				gWorld.click(world_pos, 0, 1);
				lbutton_down = false;
			}
			if (Input::is_mouse_pressed(SDL_BUTTON_RIGHT)) {
				if (!rbutton_down) {
					// Button down
					std::pair<int, int> pos = Input::get_mouse_pos();
					Vector2f world_pos = screen_to_world(pos.first, pos.second);
					gWorld.click(world_pos, 1, 0);
					rbutton_down = true;
				}
			}
			else if (rbutton_down) {
				// Button released
				std::pair<int, int> pos = Input::get_mouse_pos();
				Vector2f world_pos = screen_to_world(pos.first, pos.second);
				gWorld.click(world_pos, 1, 1);
				rbutton_down = false;
			}
		}

		// Render world
		gWorld.render(gRenderer);

		// Render left mouse drag
		if (Input::has_dragbox(SDL_BUTTON_LEFT)) {
			DragBox box = Input::get_dragbox(SDL_BUTTON_LEFT);
			// Allow panning drag
			if (ldrag_start.x() < 0 && ldrag_start.y() < 0)
				ldrag_start.set(screen_to_world(box.x1, box.y1));
			Vector2f start = world_to_screen(ldrag_start);
			SDL_Rect dragbox = {
				(int)(start.x()),
				(int)(start.y()),
				(int)(box.x2-start.x()),
				(int)(box.y2-start.y())
			};
			// Send drag world coordinates to World
			Vector2f ldrag_end = screen_to_world(box.x2, box.y2);
			gWorld.select(ldrag_start, ldrag_end, 0);
#ifdef USE_SDL2_RENDERER
			// Render filled quad
			SDL_SetRenderDrawColor(gRenderer, 0x99, 0xFF, 0x99, 0x55);
			SDL_RenderFillRect(gRenderer, &dragbox);
			// Render outline quad
			SDL_SetRenderDrawColor(gRenderer, 0x99, 0xFF, 0x99, 0xFF);
			SDL_RenderDrawRect(gRenderer, &dragbox);
#endif
		}
		else {
			ldrag_start = Vector2f(-1, -1);
		}

		// Render right mouse drag
		if (Input::has_dragbox(SDL_BUTTON_RIGHT)) {
			DragBox box = Input::get_dragbox(SDL_BUTTON_RIGHT);
			// Allow panning drag
			if (rdrag_start.x() < 0 && rdrag_start.y() < 0)
				rdrag_start.set(screen_to_world(box.x1, box.y1));
			Vector2f start = world_to_screen(rdrag_start);
			SDL_Rect dragbox = {
				(int)(start.x()),
				(int)(start.y()),
				(int)(box.x2-start.x()),
				(int)(box.y2-start.y())
			};
			// Send drag world coordinates to World
			Vector2f rdrag_end = screen_to_world(box.x2, box.y2);
			gWorld.select(rdrag_start, rdrag_end, 1);
#ifdef USE_SDL2_RENDERER
			// Render filled quad
			SDL_SetRenderDrawColor(gRenderer, 0x77, 0xAA, 0xFF, 0x55);
			SDL_RenderFillRect(gRenderer, &dragbox);
			// Render outline quad
			SDL_SetRenderDrawColor(gRenderer, 0x77, 0xAA, 0xFF, 0xFF);
			SDL_RenderDrawRect(gRenderer, &dragbox);
#endif
		}
		else {
			rdrag_start = Vector2f(-1, -1);
		}
	}

	void show() {
		// Render stats
		Stat::render(gRenderer, gFont);
#ifdef USE_SDL2_RENDERER
		// Update screen
		SDL_RenderPresent(gRenderer);
#else
		SDL_GL_SwapWindow(gWindow);
#endif
		// Free stat rendering resources
		Stat::free();
	}

	bool initialize() {
		// Initialize SDL
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			printf("SDL could not initialize: %s\n", SDL_GetError());
			return false;
		}

		// Initialize SDL_ttf
		if(TTF_Init() == -1) {
			printf("SDL_ttf could not initialize: %s\n", TTF_GetError());
			return false;
		}

		// Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
			printf("Warning: Linear texture filtering not enabled");

		// Create window
		gWindow = SDL_CreateWindow(
			"Title",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			width,
			height,
			SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
		);
		if (gWindow == NULL) {
			printf("Window could not be created: %s\n", SDL_GetError());
			return false;
		}

		// Create renderer for window
		gRenderer = SDL_CreateRenderer(
			gWindow,
			-1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
		);
		if (gRenderer == NULL) {
			printf("Renderer could not be created: %s\n", SDL_GetError());
			return false;
		}

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

		// Create a OpenGL context on SDL2
		SDL_GLContext gl_context = SDL_GL_CreateContext(gWindow);

		// Load GL extensions using glad
		if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
			std::cerr << "Failed to initialize the OpenGL context." << std::endl;
			exit(1);
		}

		// Create font from TrueType Font file
		gFont = TTF_OpenFont(
			TTF_FILE,
			FONT_SIZE
		);
		if (gFont == NULL) {
			printf("Failed to load font: %s\n", TTF_GetError());
			return false;
		}

		// Grab mouse
		// SDL_SetWindowGrab(gWindow, SDL_TRUE);

#ifdef USE_SDL2_RENDERER
		// Set blending
		SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
#else
		ogl_prepare_generic_2d_shaders(0);
		ogl_reserve_line_objects(5000);
		ogl_reserve_rect_objects(5000);
#endif
		// Opaque blank screen
		clear();
		show();

		return true;
	}

	void destroy() {
		// Free global font
		TTF_CloseFont(gFont);
		gFont = NULL;

		// Destroy renderer
		SDL_DestroyRenderer(gRenderer);
		gRenderer = NULL;

		// Destroy window
		SDL_DestroyWindow(gWindow);
		gWindow = NULL;

		// Quit SDL subsystems
		TTF_Quit();
		SDL_Quit();
	}
};

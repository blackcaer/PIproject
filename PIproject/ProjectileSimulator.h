#pragma once
#define _USE_MATH_DEFINES
#define TOLERANCE_F 1e-6


#include <vector>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>
#include "Projectile.h"
#include "Dir.h"
#include "Widget.h"

class ProjectileSimulator
{
public:
	bool running = true;

	ProjectileSimulator();
	
	void game_loop();


private: 
	sf::Font font_main;

	sf::RenderWindow window;
	sf::View view_game;
	sf::View view_controls;
	sf::RectangleShape ground;
	sf::RectangleShape rect00;
	Projectile ball;
	std::vector <Widget*> widgets;


	float VIEW_CHANGE = 10.f;
	float unit_to_px = 100.f;
	bool simulate_movement = false;
	sf::Time deltaTime;
	sf::Clock deltaClock;

	//tracer
	const float TRACER_RADIUS = 4.f;
	std::vector <sf::CircleShape*> tracers;
	float time_for_tracer_s = 0.0f;
	float tracer_interval = 0.1f;
	sf::Color tracer_color = sf::Color::Blue;

	int focus_number = -1; 

	float radius;
	float h;
	float angle;
	float v_start;
	float vx;
	float vy;
	float g;
	float ax;
	float ay;

	int counter = 0;
	float sum = 0.f;

	void prep_text(sf::Text* text);

	void print_info_to_console();

	void handle_event(sf::Event);

	void reset();

	void center_view();

	bool is_collision(float y);

	bool check_handle_collision(float*, float*);

	void move();

	void trace();

	void draw_widget(Widget*);

	void create_widgets();


	
};


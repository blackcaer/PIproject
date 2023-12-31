﻿#include "ProjectileSimulator.h"


ProjectileSimulator::ProjectileSimulator(ProjectileSimulatorArgs args) :
	START_radius(args.ball_radius),
	unit_to_px(args.unit_to_px),
	VIEW_CHANGE(args.view_change),
	TRACER_RADIUS(args.tracer_radius),
	COLOR_BALL(args.color_ball),
	COLOR_BACKGROUND(args.color_background),
	COLOR_GROUND(args.color_ground),
	STATS_FILE(args.stats_file),

	window(sf::VideoMode(1536, 960), "FluidSimulator", sf::Style::Default, sf::ContextSettings(32)),
	view_game(sf::FloatRect(0, 0, 1080, 720)),//sf::Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f), sf::Vector2f(1280,720)), 
	view_controls(sf::Vector2f(0, 0), sf::Vector2f(1280, 192)),
	ground(sf::Vector2f(400000, 200000)),
	start_marker(sf::Vector2f(5, 5))
{
	window.setPosition(sf::Vector2i(120, 20));
	radius = START_radius;
	h_start = START_h;
	angle = START_angle;
	v_start = START_v_start;
	g = START_g;

	if (!font_main.loadFromFile("arial.ttf"))
	{
		printf("FONT ERROR");
		exit(0);
	}

	tracers = std::vector <sf::CircleShape*>();
	widgets_in = std::vector <Widget*>();
	window.setVerticalSyncEnabled(true);
	window.setActive(true);

	window.clear(sf::Color(255, 0, 0));

	//======== Shapes
	top_bar = sf::RectangleShape(sf::Vector2f(1280, 300));
	top_bar.setFillColor(sf::Color(90, 90, 90));
	top_bar.setPosition(-640, -100);

	ground.setFillColor(COLOR_GROUND);
	ground.setPosition(-70000, 0);

	start_marker.setFillColor(sf::Color::Blue);
	start_marker.setPosition(0, 0);

	create_widgets();

	this->ball = Projectile(radius, 0.f, h_start + radius, COLOR_BALL);
	ball.set_zero_coordinates();
	reset();

	view_controls.setViewport(sf::FloatRect(0, 0, 1, 0.2));
	view_game.setViewport(sf::FloatRect(0, 0.2, 1, 1));

}

void ProjectileSimulator::_save_params_to_file()
{
	std::ofstream myfile;
	myfile.open(STATS_FILE, std::ofstream::app);

	myfile << "v_start: ";
	myfile << v_start;
	myfile << "; ";
	myfile << "angle: ";
	myfile << angle;
	myfile << "; ";
	myfile << "h_start: ";
	myfile << h_start;
	myfile << "; ";
	myfile << "g: ";
	myfile << g;
	myfile << "; ";
	myfile << "Z: ";
	myfile << Z;
	myfile << "; ";
	myfile << "Hmax: ";
	myfile << Hmax;
	myfile << "; ";
	myfile << "th: ";
	myfile << th;
	myfile << "; ";
	myfile << "\n";
	myfile.close();
}

void ProjectileSimulator::_prep_text(sf::Text* text, int size, sf::Color color)
{
	text->setFont(font_main);

	text->setCharacterSize(size); // in pixels, not points
	//text->setPosition(200, 100);

	text->setFillColor(color);

	//text->setStyle(sf::Text::Bold | sf::Text::Underlined);
}

void ProjectileSimulator::_print_info_to_console()
{
	_sec_elapsed += deltaTime.asSeconds();
	// FPS counter
	if (_sec_elapsed >= 1.f)
	{
		_sec_elapsed = 0;
		printf("==========\nx=%d, y=%d\n\n",
			int(ball.getShape()->getPosition().x),
			(int)(ball.getShape()->getPosition().y));

		printf("vx=%d, vy=%d\n", (int)vx, (int)vy);
	}
}

float ProjectileSimulator::deg_to_rad(float angle_deg)
{
	return angle_deg / 180.f * M_PI;
}

void ProjectileSimulator::center_view()
{
	view_game.setCenter(ball.getShape()->getPosition());
}

bool ProjectileSimulator::is_collision(float y)
// Checks if ball on coordinate y is colliding the ground
{
	if (y >= ground.getPosition().y)
	{
		return true;
	}
	return false;
}

bool ProjectileSimulator::handle_collision(float* xoffset, float* yoffset)
{
	// Check for colisions
	auto position = ball.getShape()->getPosition();

	if (is_collision(position.y + radius + *yoffset))
	{
		float to_ground_y = fabs(ground.getPosition().y - position.y);

		/*if (fabs(vx) <= TOLERANCE_F)
		{
			//ball.move(0.f, to_ground_y - radius);
			*xoffset = 0.f;
			*yoffset = to_ground_y - radius;
		}
		else // Xdddd to ogółem jakiś nieśmieszny żart TO POWINNO DZIAŁAĆ
		{
			float aoa = atan(vy / vx); // angle of attack
			float to_ground_x = to_ground_y / tan(aoa);
			//ball.move(to_ground_x - radius, to_ground_y - radius);
			//*xoffset = to_ground_x - radius;

			*xoffset = 0;	// hehe fix
			*yoffset = to_ground_y - radius;
		}*/

		*xoffset = 0;
		*yoffset = to_ground_y - radius;
		simulate_movement = false;
		vx = 0.f;
		vy = 0.f;
		return true;
	}
	else {
		// normal movement, xoffset and yoffset should remain the same
		return false;
	}
}

void ProjectileSimulator::trace()
{
	sf::CircleShape* tracer = new sf::CircleShape(TRACER_RADIUS);
	tracer->setFillColor(tracer_color);
	tracer->setOrigin(TRACER_RADIUS, TRACER_RADIUS);
	tracer->setPosition(this->ball.getShape()->getPosition());
	tracers.push_back(tracer);
}

void ProjectileSimulator::_draw_widget(Widget* widget)
{
	for (int i = 0; i < widget->to_draw.size(); i++)
	{
		window.draw(*(widget->to_draw[i]));
	}
}

void ProjectileSimulator::draw_widgets()
{
	window.setView(view_controls);
	window.draw(top_bar);
	for (auto i = 0; i < widgets_in.size(); i++)
	{
		_draw_widget(widgets_in[i]);
	}

	for (auto i = 0; i < widgets_other.size(); i++)
	{
		_draw_widget(widgets_other[i]);
	}

	for (auto i = 0; i < widgets_static.size(); i++)
	{
		_draw_widget(widgets_static[i]);
	}
	window.setView(view_game);
}

void ProjectileSimulator::create_widgets()
{
	// ============== Widgets for entering data
	auto space = 170;
	auto startx = -630;
	int height = 96;
	widgets_in.clear();
	Widget* widget_v0 = new Widget(startx, -5 - height, 160, height, "V0=");
	Widget* widget_alpha = new Widget(startx + space * 1, -5 - height, 160, height, "alfa=");
	Widget* widget_h = new Widget(startx + space * 2, -5 - height, 160, height, "h=");
	Widget* widget_g = new Widget(startx + space * 3, -5 - height, 160, height, "g=");

	widget_v0->bind_variable(&v_start);
	widget_alpha->bind_variable(&angle);
	widget_h->bind_variable(&h_start);
	widget_g->bind_variable(&g);

	widget_v0->update_widget();
	widget_alpha->update_widget();
	widget_h->update_widget();
	widget_g->update_widget();

	//Widget* widget4 = new Widget(startx + space * 4, -40, 160, 80,"?=");
	//Widget* widget5 = new Widget(startx + space * 5, -40, 160, 80,"?=");
	//Widget* widget6 = new Widget(startx + space * 6, -40, 160, 80,"?=");

	widgets_in.push_back(widget_v0);
	widgets_in.push_back(widget_alpha);
	widgets_in.push_back(widget_h);
	widgets_in.push_back(widget_g);
	//widgets.push_back(widget4);
	//widgets.push_back(widget5);
	//widgets.push_back(widget6);


	// ============== Widgets for displaying real-time data
	Widget* widget_x = new Widget(startx, 5, 160, height, "X=");
	Widget* widget_y = new Widget(startx + space * 1, 5, 160, height, "Y=");
	Widget* widget_vx = new Widget(startx + space * 2, 5, 160, height, "Vx=");
	Widget* widget_vy = new Widget(startx + space * 3, 5, 160, height, "Vy=");

	widgets_other.push_back(widget_x);
	widgets_other.push_back(widget_y);
	widgets_other.push_back(widget_vx);
	widgets_other.push_back(widget_vy);

	widget_x->bind_variable(&ball_x);
	widget_y->bind_variable(&ball_y);
	widget_vx->bind_variable(&vx);
	widget_vy->bind_variable(&vy);

	// ============== Widgets for displaying static data

	Widget* widget_Z = new Widget(20 + startx + space * 4, -5 - height, 160, height, "Zmax=");
	Widget* widget_Hmax = new Widget(20 + startx + space * 5, -5 - height, 160, height, "Hmax=");
	Widget* widget_th = new Widget(20 + startx + space * 6, -5 - height, 160, height, "th=");
	//Widget* widget_vy = new Widget(startx + space * 3, 0, 160, height, "Vy=");

	widgets_static.push_back(widget_Z);
	widgets_static.push_back(widget_Hmax);
	widgets_static.push_back(widget_th);
	//widgets_other.push_back(widget_vy);

	widget_Z->bind_variable(&Z);
	widget_Hmax->bind_variable(&Hmax);
	widget_th->bind_variable(&th);

}

void ProjectileSimulator::update_static_widgets()
{
	for (int i = 0; i < widgets_static.size(); i++)
	{
		widgets_static[i]->update_widget();
	}
}

void ProjectileSimulator::update_real_time_widgets()
{
	ball_x = ball.getShape()->getPosition().x - ball.x_zero;
	ball_y = ball.getShape()->getPosition().y - ball.y_zero;
	for (int i = 0; i < widgets_other.size(); i++)
	{
		widgets_other[i]->update_widget();
	}
}

void ProjectileSimulator::reset()
{
	vx = v_start * cos(deg_to_rad(angle)) * Dir::right;
	vy = v_start * sin(deg_to_rad(angle)) * Dir::up;
	ax = 0.f;
	ay = g * Dir::down;

	ball.getShape()->setPosition(0.f, Dir::up * (h_start * unit_to_px + radius));
	ball.set_zero_coordinates();
	if (follow_ball)
		center_view();

	// Calculate static variables
	Z = vx * ((fabs(vy) + sqrt(vy * vy + 2.f * fabs(ay) * h_start)) / ay);
	Hmax = h_start + fabs(vy * vy / (2 * ay));
	th = fabs(vy / ay);

	// Delete tracers
	for (auto i = 0; i < tracers.size(); i++)
		delete tracers[i];
	tracers.clear();

	update_static_widgets();
	simulate_movement = false;
}

void ProjectileSimulator::handle_moving_view(sf::Event event)
{
	auto center = view_game.getCenter();
	if (event.key.code == sf::Keyboard::Up)
		view_game.setCenter(center.x, center.y + VIEW_CHANGE * Dir::up);
	if (event.key.code == sf::Keyboard::Down)
		view_game.setCenter(center.x, center.y + VIEW_CHANGE * Dir::down);
	if (event.key.code == sf::Keyboard::Right)
		view_game.setCenter(center.x + VIEW_CHANGE * Dir::right, center.y);
	if (event.key.code == sf::Keyboard::Left)
		view_game.setCenter(center.x + VIEW_CHANGE * Dir::left, center.y);
}

void ProjectileSimulator::handle_tab()
{
	if (simulate_movement)
		return;
	if (focus_number == -1)
	{
		focus_number = 0;
		widgets_in[0]->toggle_focus();
	}
	else
	{
		widgets_in[focus_number]->update_variable(); // update previous widget
		widgets_in[focus_number]->toggle_focus(); // turn it OFF

		focus_number++;

		if (focus_number >= widgets_in.size())
		{
			focus_number = -1;	// This was last widget, set to -1
			reset(); // So you don't have to start two times for changes to take place
		}
		else
			widgets_in[focus_number]->toggle_focus(); // turn ON new widget


	}
}

void ProjectileSimulator::handle_letters(sf::Event event)
{
	if (event.text.unicode == 's')
	{
		simulate_movement = true;
		_save_params_to_file();
	}
	if (event.text.unicode == 'x')
		view_game.zoom(2.f);
	if (event.text.unicode == 'z')
		view_game.zoom(0.5f);
	if (event.text.unicode == 'c')
		view_game.setCenter(ball.getShape()->getPosition());
	if (event.text.unicode == 'r')
	{
		ball.getShape()->setPosition(0.f, radius * Dir::up);
		reset();
	}
	if (event.text.unicode == 'v')
		follow_ball = !follow_ball;

	if (event.text.unicode == 'g')
	{
		simulate_movement = false;
	}
}

void ProjectileSimulator::handle_entering_numbers(sf::Event event)
{
	if (focus_number == -1)
		return;
	auto current_text = widgets_in[focus_number]->get_user_text();
	if (
		event.text.unicode >= 48 and
		event.text.unicode <= 57
		)
	{
		widgets_in[focus_number]->set_user_text(current_text + (char)(event.text.unicode));
	}
	else if (event.text.unicode == '.')
	{
		if (current_text.find('.') != std::string::npos)
			return;
		widgets_in[focus_number]->set_user_text(current_text + (char)(event.text.unicode));
	}
}

void ProjectileSimulator::handle_event(sf::Event event)
{
	switch (event.type){
	case sf::Event::Closed:
		running = false;
		break;

	case sf::Event::KeyPressed:

		handle_moving_view(event);

		if (event.key.code == sf::Keyboard::Tab)
			handle_tab();

		if (focus_number != -1 and (
				event.key.code == sf::Keyboard::BackSpace or
				event.key.code == sf::Keyboard::Delete))
		{
			widgets_in[focus_number]->delete_last_char();
		}
		break;

	case sf::Event::TextEntered:

		if (focus_number == -1)
			handle_letters(event);
		else
			handle_entering_numbers(event);
		break;
	}
}

void ProjectileSimulator::move()
{
	float dt = deltaTime.asSeconds();

	float xoffset = (vx * dt + ax * dt * dt / 2.f) * unit_to_px;
	float yoffset = (vy * dt + ay * dt * dt / 2.f) * unit_to_px;

	vx += ax * dt;
	vy += ay * dt;

	handle_collision(&xoffset, &yoffset);

	ball.move(xoffset, yoffset);

	if (follow_ball)
		center_view();
}

void ProjectileSimulator::game_loop()
{
	float time_for_tracer = 0.0f;
	sf::Event event;

	while (running){
		deltaTime = deltaClock.restart();

		while (window.pollEvent(event))
			handle_event(event);

		_print_info_to_console();

		if (simulate_movement)
		{
			move();
			time_for_tracer += deltaTime.asSeconds();
			if (time_for_tracer >= tracer_interval)
			{
				time_for_tracer = 0.f;
				trace();
			}
		}
		update_real_time_widgets();

		//============ Drawing 
		window.clear(COLOR_BACKGROUND);
		window.setView(view_game);

		// Widgets
		draw_widgets();

		// Tracers
		for (auto i = 0; i < tracers.size(); i++)
			window.draw(*tracers[i]);

		window.draw(ground);
		window.draw(start_marker);
		window.draw(*(ball.getShape()));
		window.display();
	}

}


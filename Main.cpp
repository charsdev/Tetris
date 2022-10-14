#include <SFML/Graphics.hpp>
#include <random>

const int COLUMN = 26;
const int ROW = 18;
const int BLOCKSIZE = 18;
const int BLOCKAMOUNT = 4;
const int MAXBLOCK_TILES = 7;
const float INITIAL_DELAY = 0.45f;
const int w = 320;
const int h = 466;

bool escapeButtonStatus = false;
bool pause = false;
unsigned score = 0;
bool gameOver = false;

int field[COLUMN][ROW] = { 0 };
sf::Vector2i current[4];
sf::Vector2i next[4];

int figures[7][4] =
{
	1, 3, 5, 7,
	2, 4, 5, 7,
	3, 5, 4, 6,
	3, 5, 4, 7,
	2, 3, 5, 7,
	3, 5, 7, 6,
	2, 3, 4, 5,
};

void Move(int dx, int dy)
{
	for (int i = 0; i < BLOCKAMOUNT; i++)
	{
		next[i] = current[i];
		current[i].x += dx;
		current[i].y += dy;
	}
}

bool Check()
{
	for (const auto& block : current)
	{
		if (block.x < 0 || block.x >= ROW || block.y >= COLUMN)
		{
			return false;
		}
		if (field[block.y][block.x])
		{
			return false;
		}
	}
	return true;
}

template<typename T>
T random(T range_from, T range_to) {
	std::random_device rand_dev;
	std::mt19937                        generator(rand_dev());
	std::uniform_int_distribution<T>    distr(range_from, range_to);
	return distr(generator);
}

void ReplaceArr(sf::Vector2i* a, const sf::Vector2i* b)
{
	for (size_t i = 0; i < BLOCKAMOUNT; i++)
	{
		a[i] = b[i];
	}
}

void Rotate()
{
	sf::Vector2i pivot = current[1];

	for (int i = 0; i < BLOCKAMOUNT; i++)
	{
		int x = current[i].y - pivot.y;
		int y = current[i].x - pivot.x;
		current[i].x = pivot.x - x;
		current[i].y = pivot.y + y;
	}

	if (!Check())
	{
		ReplaceArr(current, next);
	}
}

void CheckLines()
{
	int k = COLUMN - 1;
	for (int i = COLUMN - 1; i > 0; i--)
	{
		int count = 0;
		for (int j = 0; j < ROW; j++)
		{
			if (field[i][j])
			{
				count++;
			}
			field[k][j] = field[i][j];
		}

		if (count < ROW)
		{
			k--;
		}
		else
		{
			++score;
		}
	}
	if (k != 0)
	{
		std::fill(field[0], field[0] + (ROW * (k + 1)), 0);
	}
}


int main()
{
	srand(time(0));

	//Init
	sf::RenderWindow window(sf::VideoMode(w, h), "TETRIS SFML");
	sf::Texture textureBlock;

	if (!textureBlock.loadFromFile("Images/tiles.png"))
	{
		return 0;
	}

	sf::Sprite spriteBlock(textureBlock);
	spriteBlock.setTextureRect(sf::IntRect(0, 0, BLOCKSIZE, BLOCKSIZE));

	sf::Font font;
	if (!font.loadFromFile("fonts/retro.ttf"))
	{
		return 0;
	}

	sf::Text text;
	text.setCharacterSize(14); // in pixels, not points!
	text.setFont(font);
	text.setFillColor(sf::Color::White);


	//variables
	int colorNum = random(0, MAXBLOCK_TILES);
	float timer = 0;
	float delay = INITIAL_DELAY;
	sf::Clock clock;

	const int firstFigure = random(0, MAXBLOCK_TILES);
	for (int i = 0; i < BLOCKAMOUNT; i++)
	{

		current[i].x = figures[firstFigure][i] % 2;
		current[i].y = figures[firstFigure][i] / 2;
	}

	// Loop
	while (window.isOpen())
	{
		text.setString("Score " + std::to_string(score));

		if (!pause || !escapeButtonStatus)
		{
			timer += clock.restart().asSeconds();
		}

		//Input
		sf::Event e;
		while (window.pollEvent(e))
		{
			if (e.type == sf::Event::Closed)
			{
				window.close();
			}

			if (e.type == sf::Event::LostFocus)
				pause = true;

			if (e.type == sf::Event::GainedFocus)
				pause = false;

			//Keys Input
			if (e.type == sf::Event::KeyReleased)
			{
				if (e.key.code == sf::Keyboard::Up)
				{
					Rotate();
				}
				else if (e.key.code == sf::Keyboard::Left)
				{
					Move(-1, 0);
				}
				else if (e.key.code == sf::Keyboard::Right)
				{
					Move(1, 0);
				}
				else if (e.key.code == sf::Keyboard::Escape)
				{
					escapeButtonStatus = !escapeButtonStatus;
				}
			}
		}

		if (pause || escapeButtonStatus)
			continue;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
		{
			delay = 0.05f;
		}

		if (!Check())
		{
			ReplaceArr(current, next);
		}

		//tick
		if (timer > delay)
		{
			Move(0, 1);

			if (!Check())
			{
				for (const auto& v : next)
				{
					field[v.y][v.x] = colorNum;
				}

				colorNum = random(0, MAXBLOCK_TILES);
				int n = random(0, MAXBLOCK_TILES);
				for (int i = 0; i < BLOCKAMOUNT; i++)
				{
					current[i].x = figures[n][i] % 2;
					current[i].y = figures[n][i] / 2;
				}
				if (!Check())
				{
					gameOver = true;
				}
			}

			timer = 0;
		}

		if (gameOver) {
			window.clear(sf::Color::Black);
			text.setString("Game Over");
			sf::FloatRect textRect = text.getLocalBounds();
			text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
			text.setPosition(window.getView().getCenter());
			window.draw(text);
			window.display();
			continue;
		}

		CheckLines();

		delay = INITIAL_DELAY;
		window.clear(sf::Color::Black);

		//Draw block high
		for (int i = 0; i < COLUMN; i++)
		{
			for (int j = 0; j < ROW; j++)
			{
				int point = field[i][j];

				if (point == 0)
				{
					continue;
				}

				spriteBlock.setTextureRect(sf::IntRect(point * BLOCKSIZE, 0, BLOCKSIZE, BLOCKSIZE));
				spriteBlock.setPosition(j * BLOCKSIZE, i * BLOCKSIZE);
				window.draw(spriteBlock);
			}
		}

		//Draw block low
		for (int i = 0; i < BLOCKAMOUNT; i++)
		{
			spriteBlock.setTextureRect(sf::IntRect(colorNum * BLOCKSIZE, 0, BLOCKSIZE, BLOCKSIZE));
			spriteBlock.setPosition(current[i].x * BLOCKSIZE, current[i].y * BLOCKSIZE);
			window.draw(spriteBlock);
		}

		window.draw(text);
		window.display();
	}
	return 0;
}
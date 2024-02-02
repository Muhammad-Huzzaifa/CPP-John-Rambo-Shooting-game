#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include <Windows.h>
#include <SFML/Audio.hpp>

using namespace std;

// Dimensions
const int HEIGHT = 51;                                            // Including border lines that takes 2 extra rows
const int WIDTH = 32;                                             // Including border lines that takes 2 extra columns
const int UP = 72, DOWN = 80, LEFT = 75, RIGHT = 77, SPACE = 32;  // Keys ASCII values to control Rambo movement
const int SPEED_ROAD = WIDTH / 2;                                 // When Rambo is on the speed road the speed becomes doubles
int LIVES_RAMBO = 1;                                              // Remaining lives of Rambo
int SCORE_GAME = 0;                                               // Score of the game 
int STEPS_RAMBO = 0;                                              // Steps in meters, 1step is equal to 0.5meter hence 2steps is 1meter
int LEVEL = 1;                                                    // Initially level_1, when Rambo moves 5km then level_2
int GAME_SPEED = 200;                                             // Speed of the game depends on the sleep duration
int NO_OBS_CRT = 0;                                               // Purpose to maintain obstacles and crates in the map
int CRT_DESTROYED = 0;                                            // Purpose to check if Rambo destroyed 5 crates it gains 1 extra life
int NO_TANKS = 0;                                                 // Purpose to maintain tanks in the map
int TANK_DESTROYED = 0;                                           // Purpose to check if Rambo destroyed 10 tanks which cause the start of level_2
char gameMap[HEIGHT][WIDTH + 1];                                  // 2D character array representing the game map

// Function Prototypes
void hideCursor();                                                           // Function to hide the console cursor
void clearScreen();                                                          // Function to clear the screen when the map is updated
void initialize_Map(bool&, int&, int&);                                      // Initialize the game map
void Game_Bar();                                                             // Top game bar shows nessacary info about the game's current status
void control_Level_Change();                                                 // Increases the number of steps when a key is pressed
void display_Map();                                                          // Display the game map
void movement_Map();                                                         // Move the game map
void create_Rambo(int, int, bool&);                                          // To shape and spawn Rambo in the map
void bullet_Fire_Rambo(int&, int&);                                          // Generate bullets from Rambo according to its position
void bullet_Update_Rambo();                                                  // To update the position of Rambo's bullets while considering map movement
void move_Rambo(bool&, bool&, int&, int&, char);                             // Movement of Rambo according to the user pressed keys
void collision_Rambo(int&, int&);                                            // To check the collision of Rambo with tank's bullets, tanks, obstacles, and crates
bool validPosition_Obs_Crt(int, int);                                        // Check if position is valid for placing obstacle or crate
void spawn_Obs_Crt();                                                        // Randomly spawn obstacles and crates
void check_LastRow();                                                        // Check if obstacle and crate is in the last row
void spawn_Obs_Crt_Again();                                                  // Spawn obstacle and crate again if they are less than allowed obstacles & crates in different levels
void create_Tank(int, int);                                                  // Shape a tank and creation
bool validPosition_Tank(int, int);                                           // Check if position is valid for placing a tank
void spawn_Tank(int, int, int, int);                                         // Randomly spawn a tank
void spawn_Tank_Again();                                                     // Spawn the tank again if they are less than allowed tanks in different levels
void extra_Movements();                                                      // Updates bullet positions while accounting for map movement

int main()
{
	srand(time(0));
	int random = rand() % 2;
	// To add music in the game
	sf::Music music;
	if (random == 0)
		music.openFromFile("Game_sound1.wav");
	else
		music.openFromFile("Game_sound2.wav");
	music.setLoop(true);
	music.play();

	const int size = 10;
	// String array to store the names in the text file
	string names[size];
	// Array that stores the top_10 high scores and manage it in the text file
	int scores[size];
	// Input and output file objects
	ifstream infile;
	ofstream outfile;

	char key = '\0';
	int Rambo_y = HEIGHT - 3, Rambo_x = WIDTH / 2;
	bool game_end = false;
	bool jump = false;

	initialize_Map(jump, Rambo_x, Rambo_y);                        // Initialize the game map
	spawn_Obs_Crt();                                               // Create obstacles and crates at random positions
	spawn_Tank(0, 0, 0, 0);                                        // Create a tank at a random position
	display_Map();                                                 // Display the initial game map

	while (!game_end && LIVES_RAMBO != 0)
	{
		SCORE_GAME += 1;                                   // The score of the game increases at every step by 1
		if (LEVEL == 1)
		{
			if (Rambo_x == SPEED_ROAD)
				Sleep((GAME_SPEED * 1.5) / 2);             // In the speed road the speed is doubled what in level_1
			else
				Sleep(GAME_SPEED * 1.5);                   // In level_1 the sleep time is 1.5x more than that in level_2
		}
		else
		{
			if (Rambo_x == SPEED_ROAD)
				Sleep(GAME_SPEED / 2);                     // In the speed road the speed is doubled what in level_2
			else
				Sleep(GAME_SPEED);                         // In level_2 the sleep time is 1.5x less than that in level_1
		}

		create_Rambo(Rambo_x, Rambo_y, jump);              // Create Rambo while considering the jump shape and original shape and clear its previous position
		move_Rambo(game_end, jump, Rambo_x, Rambo_y, key); // Move the Rambo and actions according to the user pressed keys
		movement_Map();                                    // Move the map by taking all the rows move down by one
		control_Level_Change();

		if (key == 's' || key == 'S')
			bullet_Fire_Rambo(Rambo_x, Rambo_y);           // Fire the bullet from Rambo when "s" key is pressed
		else
			bullet_Update_Rambo();                         // If the key is not pressed then only update the position of bullet

		collision_Rambo(Rambo_x, Rambo_y);                 // If the Rambo collide anything its life decreases by one

		extra_Movements();                                 // Controlling the bullet movement of tank and Rambo according to map movement
	}

	clearScreen();
	display_Map();                                         // To print the final map before the end of the game

	// Open the High Score file for filling the score array
	infile.open("HIGH SCORES.txt");
	if (!infile.is_open())
		cout << "Error occurs in opening the high score text file for reading";
	else
	{
		for (int i = 0; i < size; i++)
		{
			infile >> names[i];  // Reading names from text file and stores it in the names array
			infile >> scores[i]; // Reading scores from text file and store it in the score array
		}
	}
	infile.close();


	// Operations on the array that modify the top 10 scores
	int high = SCORE_GAME, index = -1;
	for (int i = 0; i < size; i++)
	{
		if (scores[i] <= high)
		{
			index = i;
			break;
		}
	}
	if (index != -1) // If any High score found
	{
		for (int i = size - 1; i > index; i--)
		{
			scores[i] = scores[i - 1];
		}
		scores[index] = high;
	}

	// Open the text file for writing the modified array of High Scores
	outfile.open("HIGH SCORES.txt");
	if (!outfile.is_open())
		cout << "Error occurs in opening the high score text file for writing";
	else
	{
		for (int i = 0; i < size; i++)
			outfile << names[i] << " " << scores[i] << endl;
	}
	outfile.close();


	music.pause(); // Stop the music when the game ends

	system("pause");
	return 0;
}

// Function to hide the console cursor
void hideCursor()
{
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info{};
	info.dwSize = HEIGHT * WIDTH;
	info.bVisible = FALSE;
	SetConsoleCursorInfo(consoleHandle, &info);
}

// Function to clear the screen when the map is updated
void clearScreen()
{
	system("cls");
}

// Initialize the game map with borders and empty spaces and the Rambo
void initialize_Map(bool& jump, int& Rambo_x, int& Rambo_y)
{
	for (int i = 0; i < HEIGHT; i++)
	{
		for (int j = 0; j < WIDTH; j++)
		{
			if (i == Rambo_y && j == Rambo_x)
			{
				gameMap[i][j] = '_';
				if (jump == true)
				{
					gameMap[i][j] = 'V';
					jump = false;
				}
			}
			else if (i == Rambo_y && j == Rambo_x - 1)
				gameMap[i][j] = '^';
			else if (i == Rambo_y && j == Rambo_x + 1)
				gameMap[i][j] = '^';
			else if ((i == 0 && j != SPEED_ROAD) || (i == HEIGHT - 1 && j != SPEED_ROAD) || j == 0 || j == WIDTH - 1)
				gameMap[i][j] = '.';
			else if ((i == 0 && j == SPEED_ROAD) || (i == HEIGHT - 1 && j == SPEED_ROAD))
				gameMap[i][j] = 's';
			else
				gameMap[i][j] = ' ';
		}
	}
}

// Game Bar that contains info about the current status of the game which is on the top
void Game_Bar()
{
	char Lives[] = { 'L','I','V','E','S',':',' ','\0' };
	char Score[] = { 'S','C','O','R','E',':',' ','\0' };
	char Level[] = { 'L','E','V','E','L',':',' ','\0' };
	char Distance[] = { 'D','I','S','T','A','N','C','E',':',' ','\0' };

	cout << Lives;
	for (int i = 0; i < LIVES_RAMBO; i++)
	{
		cout << (char)3;  // To print heart character that tells us the the remaining lives of Rambo
	}
	cout << setw(13) << Score << setw(3) << SCORE_GAME << setw(15) << Distance << setw(4) << STEPS_RAMBO << 'm' << setw(15) << Level << LEVEL << endl;
}

// Function to increase number of steps considering every movement of Rambo
void control_Level_Change() {
	STEPS_RAMBO += 2;        // Every step increment cause the increase of 2m distance
	if (STEPS_RAMBO >= 5000) // If steps greater than 5km than update level_1 into level_2
		LEVEL = 2;
	else if (TANK_DESTROYED >= 10)
		LEVEL = 2;
}

// Display the current state of the game map
void display_Map()
{
	Game_Bar();
	for (int i = 0; i < HEIGHT; i++)
	{
		hideCursor();
		for (int j = 0; j < WIDTH; j++)
		{
			hideCursor();
			if (gameMap[i][j] == '.')
				// Blue colour
				cout << "\033[34m" << gameMap[i][j] << " ";

			else if (gameMap[i][j] == 's')
				// Bold_Red colour
				cout << "\033[1m\033[31m" << gameMap[i][j] << " ";

			else if (gameMap[i][j] == '^' || gameMap[i][j] == '_' || gameMap[i][j] == 'V')
				// Cayan colour
				cout << "\033[36m" << gameMap[i][j] << " ";

			else if (gameMap[i][j] == 'o' || gameMap[i][j] == '*')
				// Yellow colour
				cout << "\033[33m" << gameMap[i][j] << " ";

			else if (gameMap[i][j] == '|')
				// Red colour
				cout << "\033[31m" << gameMap[i][j] << " ";

			else if (gameMap[i][j] == '-' || gameMap[i][j] == '~' || gameMap[i][j] == '[' || gameMap[i][j] == ']' || gameMap[i][j] == 'O')
				// Dark_Gray colour
				cout << "\033[90m" << gameMap[i][j] << " ";

			else if (gameMap[i][j] == '\\' || gameMap[i][j] == '/')
				// Green colour
				cout << "\033[32m" << gameMap[i][j] << " ";

			else
				// Reset the colour
				cout << "\033[0m" << gameMap[i][j] << " ";
		}
		cout << endl;
	}
}

// Move the game map by shifting the rows downwards
void movement_Map()
{
	for (int i = HEIGHT - 2; i >= 1; i--)       // Starts from the last row and take elements of previous row
	{
		if (i == 1)
		{
			for (int j = 1; j < WIDTH - 1; j++) // First row is filled with spaces
				gameMap[i][j] = ' ';
		}
		else
		{
			for (int j = 1; j < WIDTH - 1; j++)
				gameMap[i][j] = gameMap[i - 1][j];
		}
	}
	hideCursor();
	clearScreen();
	display_Map();

	// Spawns Check
	spawn_Obs_Crt_Again();
	spawn_Tank_Again();

	// Number check for obstacles/crates and tanks
	check_LastRow();
}

// Create and shape Rambo at specified position
void create_Rambo(int Rambo_x, int Rambo_y, bool& jump)
{
	// Clear the previous Rambo position
	for (int i = 0; i < HEIGHT; i++)
	{
		for (int j = 0; j < WIDTH; j++)
		{
			if (gameMap[i][j] == '_' || gameMap[i][j] == '^' || gameMap[i][j] == 'V')
			{
				gameMap[i][j] = ' ';
			}
		}
	}

	// Set the new Rambo position
	if (jump)
	{
		gameMap[Rambo_y][Rambo_x] = 'V';     // Rambo's head when jumping
		gameMap[Rambo_y][Rambo_x - 1] = '^';
		gameMap[Rambo_y][Rambo_x + 1] = '^';
		jump = false;                        // Reset the jump flag
	}
	else
	{
		gameMap[Rambo_y][Rambo_x] = '_';     // Rambo's body
		gameMap[Rambo_y][Rambo_x - 1] = '^'; // Rambo's left side
		gameMap[Rambo_y][Rambo_x + 1] = '^'; // Rambo's right side
	}
}

// Generate bullet from Rambo's above position
void bullet_Fire_Rambo(int& Rambo_x, int& Rambo_y)
{
	gameMap[Rambo_y - 1][Rambo_x] = '*';
}

// Updating position of Rambo's bullet considering map movement
void bullet_Update_Rambo()
{
	for (int i = 2; i < HEIGHT - 1; i++)
	{
		for (int j = 1; j < WIDTH; j++)
		{
			if (gameMap[i][j] == '*')
			{
				gameMap[i][j] = ' ';
				if (gameMap[i - 2][j] == ' ' && gameMap[i - 1][j] == ' ')
					gameMap[i - 2][j] = '*';

				// Detecting collision with crate, score updating and disappearing crate after collision
				if ((gameMap[i - 2][j] == '/' && gameMap[i - 2][j + 1] == '~') || (gameMap[i - 1][j] == '/' && gameMap[i - 1][j + 1] == '~'))
				{
					SCORE_GAME += 5;
					int x = j, y;
					if (gameMap[i - 1][j] == '/')
						y = i - 1;
					else if (gameMap[i - 2][j] == '/')
						y = i - 2;
					gameMap[y][x] = ' ';
					x += 1;
					while (gameMap[y][x] != '/')
					{
						gameMap[y][x] = ' ';
						x += 1;
					}
					gameMap[y][x] = ' ';
					NO_OBS_CRT--;
					CRT_DESTROYED += 1;
				}
				else if ((gameMap[i - 2][j] == '/' && gameMap[i - 2][j - 1] == '~') || (gameMap[i - 1][j] == '/' && gameMap[i - 1][j - 1] == '~'))
				{
					SCORE_GAME += 5;
					int x = j, y;
					if (gameMap[i - 1][j] == '/')
						y = i - 1;
					else if (gameMap[i - 2][j] == '/')
						y = i - 2;
					gameMap[y][x] = ' ';
					x -= 1;
					while (gameMap[y][x] != '/')
					{
						gameMap[y][x] = ' ';
						x -= 1;
					}
					gameMap[y][x] = ' ';
					NO_OBS_CRT--;
					CRT_DESTROYED += 1;
				}
				else if (gameMap[i - 2][j] == '~' || gameMap[i - 1][j] == '~')
				{
					SCORE_GAME += 5;
					int x1 = j, x2 = j, y;
					if (gameMap[i - 1][j] == '~')
						y = i - 1;
					else if (gameMap[i - 2][j] == '~')
						y = i - 2;
					gameMap[y][x1] = ' ';
					x1 += 1;
					while (gameMap[y][x1] != '/')
					{
						gameMap[y][x1] = ' ';
						x1 += 1;
					}
					gameMap[y][x1] = ' ';
					x2 -= 1;
					while (gameMap[y][x2] != '/')
					{
						gameMap[y][x2] = ' ';
						x2 -= 1;
					}
					gameMap[y][x2] = ' ';
					NO_OBS_CRT--;
					CRT_DESTROYED += 1;
				}

				if (CRT_DESTROYED == 5)
				{
					CRT_DESTROYED = 0;
					LIVES_RAMBO += 1;
				}

				// Detection collision with tank, score updating and disappearing tank after collision
				if ((gameMap[i - 2][j] == '|' && gameMap[i - 2][j - 1] == ' ' && gameMap[i - 2][j + 1] == ' ') || (gameMap[i - 1][j] == '|' && gameMap[i - 1][j - 1] == ' ' && gameMap[i - 1][j + 1] == ' '))
				{
					SCORE_GAME += 10;
					int x = j - 2, y;
					if (gameMap[i - 1][j] == '|')
						y = i - 4;
					else if (gameMap[i - 2][j] == '|')
						y = i - 5;
					for (int k = y; k <= y + 3; k++)
					{
						for (int l = x; l <= x + 4; l++)
							gameMap[k][l] = ' ';
					}
					NO_TANKS--;
					TANK_DESTROYED += 1;
				}
				else if ((gameMap[i - 2][j] == '\\' && gameMap[i - 2][j - 1] == ' ' && gameMap[i - 2][j + 1] == ' ') || (gameMap[i - 1][j] == '\\' && gameMap[i - 1][j - 1] == ' ' && gameMap[i - 1][j + 1] == ' '))
				{
					SCORE_GAME += 10;
					int x = j, y;
					if (gameMap[i - 1][j] == '\\')
						y = i - 4;
					else if (gameMap[i - 2][j] == '\\')
						y = i - 5;
					for (int k = y; k <= y + 3; k++)
					{
						for (int l = x; l <= x + 4; l++)
							gameMap[k][l] = ' ';
					}
					NO_TANKS--;
					TANK_DESTROYED += 1;
				}
				else if ((gameMap[i - 2][j] == '/' && gameMap[i - 2][j - 1] == ' ' && gameMap[i - 2][j + 1] == ' ') || (gameMap[i - 1][j] == '/' && gameMap[i - 1][j - 1] == ' ' && gameMap[i - 1][j + 1] == ' '))
				{
					SCORE_GAME += 10;
					int x = j - 4, y;
					if (gameMap[i - 1][j] == '/')
						y = i - 4;
					else if (gameMap[i - 2][j] == '/')
						y = i - 5;
					for (int k = y; k <= y + 3; k++)
					{
						for (int l = x; l <= x + 4; l++)
							gameMap[k][l] = ' ';
					}
					NO_TANKS--;
					TANK_DESTROYED += 1;
				}
				else if ((gameMap[i - 2][j] == '\\' && gameMap[i - 2][j - 1] == '|' && gameMap[i - 2][j + 1] == 'O') || (gameMap[i - 1][j] == '\\' && gameMap[i - 1][j - 1] == '|' && gameMap[i - 1][j + 1] == 'O'))
				{
					SCORE_GAME += 10;
					int x = j - 1, y;
					if (gameMap[i - 1][j] == '\\')
						y = i - 3;
					else if (gameMap[i - 2][j] == '\\')
						y = i - 4;
					for (int k = y; k <= y + 3; k++)
					{
						for (int l = x; l <= x + 4; l++)
							gameMap[k][l] = ' ';
					}
					NO_TANKS--;
					TANK_DESTROYED += 1;
				}
				else if ((gameMap[i - 2][j] == '/' && gameMap[i - 2][j - 1] == 'O' && gameMap[i - 2][j + 1] == '|') || (gameMap[i - 1][j] == '/' && gameMap[i - 1][j - 1] == 'O' && gameMap[i - 1][j + 1] == '|'))
				{
					SCORE_GAME += 10;
					int x = j - 3, y;
					if (gameMap[i - 1][j] == '/')
						y = i - 3;
					else if (gameMap[i - 2][j] == '/')
						y = i - 4;
					for (int k = y; k <= y + 3; k++)
					{
						for (int l = x; l <= x + 4; l++)
							gameMap[k][l] = ' ';
					}
					NO_TANKS--;
					TANK_DESTROYED += 1;
				}
			}
		}
	}
}

// Function to control Rambo's movement
void move_Rambo(bool& end, bool& jump, int& Rambo_x, int& Rambo_y, char key)
{
	if (_kbhit())
	{
		key = _getch();
		switch (key)
		{
		case UP:          // Move Rambo up
			Rambo_y--;
			if (Rambo_y >= (HEIGHT - 2) - 15)
				control_Level_Change();
			break;
		case DOWN:        // Move Rambo down
			if (Rambo_y <= HEIGHT - 4)
			{
				Rambo_y++;
				if (Rambo_y <= HEIGHT - 2)
					control_Level_Change();
			}
			break;
		case LEFT:        // Move Rambo left
			Rambo_x--;
			if (Rambo_x >= 2)
				control_Level_Change();
			break;
		case RIGHT:       // Move Rambo right
			Rambo_x++;
			if (Rambo_x <= WIDTH - 3)
				control_Level_Change();
			break;
		case SPACE:
			jump = true;  // Set jump flag for Rambo
			Rambo_y -= 2; // Move Rambo upward for the jump
			if (Rambo_y >= (HEIGHT - 2) - 15)
			{
				control_Level_Change();
				control_Level_Change();
			}
			else if (Rambo_y - 1 >= (HEIGHT - 2) - 15)
				control_Level_Change();
			break;
		case 'S':         // Rambo shoot a fire
		case 's':
			bullet_Fire_Rambo(Rambo_x, Rambo_y);
			break;
		case 'Q':
		case 'q':
			end = true;   // Quit the game
			break;
		default:
			break;
		}
	}

	// Restrict Rambo's movement within map boundaries and Rambo limits
	if (Rambo_y >= HEIGHT - 2)
		Rambo_y = HEIGHT - 2;
	else if (Rambo_y <= (HEIGHT - 2) - 15)
		Rambo_y = (HEIGHT - 2) - 15;

	if (Rambo_x <= 2)
		Rambo_x = 2;
	else if (Rambo_x >= WIDTH - 3)
		Rambo_x = WIDTH - 3;
}

// Function that check the collision of Rambo with tank's bullets, tanks, obstacles and crates
void collision_Rambo(int& Rambo_x, int& Rambo_y)
{
	if (gameMap[Rambo_y][Rambo_x] == 'o' || gameMap[Rambo_y][Rambo_x] == '\\' || gameMap[Rambo_y][Rambo_x] == '|' || gameMap[Rambo_y][Rambo_x] == '/' || gameMap[Rambo_y][Rambo_x] == '-' || gameMap[Rambo_y][Rambo_x] == '~')
		LIVES_RAMBO--;
	else if (gameMap[Rambo_y][Rambo_x - 1] == 'o' || gameMap[Rambo_y][Rambo_x - 1] == '\\' || gameMap[Rambo_y][Rambo_x - 1] == '|' || gameMap[Rambo_y][Rambo_x - 1] == '/' || gameMap[Rambo_y][Rambo_x - 1] == '-' || gameMap[Rambo_y][Rambo_x - 1] == '~')
		LIVES_RAMBO--;
	else if (gameMap[Rambo_y][Rambo_x + 1] == 'o' || gameMap[Rambo_y][Rambo_x + 1] == '\\' || gameMap[Rambo_y][Rambo_x + 1] == '|' || gameMap[Rambo_y][Rambo_x + 1] == '/' || gameMap[Rambo_y][Rambo_x + 1] == '-' || gameMap[Rambo_y][Rambo_x + 1] == '~')
		LIVES_RAMBO--;
	else if (gameMap[Rambo_y - 1][Rambo_x - 1] == 'o' || gameMap[Rambo_y - 1][Rambo_x] == 'o' || gameMap[Rambo_y - 1][Rambo_x + 1] == 'o')
		LIVES_RAMBO--;
}

// Check if the given position is valid to place a obstacles and crates
bool validPosition_Obs_Crt(int startCol, int width)
{
	for (int i = 1; i <= 2; i++)
	{
		for (int j = startCol; j < startCol + width; j++)
		{
			if (gameMap[i][j] != ' ')
				return false;
		}
	}
	return true;
}

// Randomly spawn obstacles and crates on the map
void spawn_Obs_Crt()
{
	srand(time(0));
	int starting_Col_Obs_Crt, width_Obs_Crt; // Starting column and width of obstacle or crate
	int chance_Crt;                          // Generate the chance of crate appearance
	int max_Obs_Crt;                         // Maximum number of obstacles and crates in different levels

	if (LEVEL == 1)
		max_Obs_Crt = 3;                     // There are maximum three obstacles/crates in level_1
	else
		max_Obs_Crt = 5;                     // There are maximum five obstacles/crates in level_2

	if (NO_OBS_CRT < max_Obs_Crt)
	{
		width_Obs_Crt = rand() % 5 + 3; // Random width for obstacles and crates
		chance_Crt = rand() % 10 + 1;   // To generate the crate

		// Starting column is the starting point of obstacle and crate, & the do-while checks that obstacle or crate don't overflow border
		do
		{
			starting_Col_Obs_Crt = rand() % (WIDTH - 4) + 1;
		} while (!validPosition_Obs_Crt(starting_Col_Obs_Crt, width_Obs_Crt));

		// Obstacle and crate creation
		for (int i = starting_Col_Obs_Crt; i < width_Obs_Crt + starting_Col_Obs_Crt; i++)
		{
			if (chance_Crt == 1 || chance_Crt == 2) // 20% chance out of 100% for crate generation
			{
				if (i == starting_Col_Obs_Crt || i == (width_Obs_Crt + starting_Col_Obs_Crt - 1))
					gameMap[1][i] = '/';
				else
					gameMap[1][i] = '~';
			}
			else if (i == starting_Col_Obs_Crt || i == (width_Obs_Crt + starting_Col_Obs_Crt - 1))
				gameMap[1][i] = '|';
			else
				gameMap[1][i] = '-';
		}
		NO_OBS_CRT++; // If one obstacle or crate i spawn increase its number to control it
	}
}

// Function to check if obstacle or crate reaches the last row
void check_LastRow()
{
	for (int i = 1; i < WIDTH - 1; i++)
	{
		if (gameMap[HEIGHT - 2][i] == '|')         // For obstacle
		{
			if (gameMap[HEIGHT - 2][i + 1] == '-')
				NO_OBS_CRT--;
		}
		else if (gameMap[HEIGHT - 2][i] == '/')   // For crate
		{
			if (gameMap[HEIGHT - 2][i + 1] == '~')
				NO_OBS_CRT--;
		}
		else if (gameMap[HEIGHT - 2][i] == '[')  // For tank
			NO_TANKS--;
	}
}

// Spawn obstacle and crate again if it has less in numbers as allowed
void spawn_Obs_Crt_Again()
{
	int spawnObsCrtAgain = rand() % 5; // Randomly generates another one if previous one diminishes
	if (spawnObsCrtAgain == 0)
		spawn_Obs_Crt();
}

// Create the shape of a tank at a specified position
void create_Tank(int row, int col)
{
	gameMap[row][col] = '/';
	gameMap[row][col + 1] = '-';
	gameMap[row][col + 2] = '-';
	gameMap[row][col + 3] = '-';
	gameMap[row][col + 4] = '\\';
	gameMap[row + 1][col] = '|';
	gameMap[row + 1][col + 1] = '[';
	gameMap[row + 1][col + 2] = '-';
	gameMap[row + 1][col + 3] = ']';
	gameMap[row + 1][col + 4] = '|';
	gameMap[row + 2][col] = '|';
	gameMap[row + 2][col + 1] = '\\';
	gameMap[row + 2][col + 2] = 'O';
	gameMap[row + 2][col + 3] = '/';
	gameMap[row + 2][col + 4] = '|';
	gameMap[row + 3][col] = '\\';
	gameMap[row + 3][col + 1] = ' ';
	gameMap[row + 3][col + 2] = '|';
	gameMap[row + 3][col + 3] = ' ';
	gameMap[row + 3][col + 4] = '/';
}

// Check if the given position is valid to place a tank
bool validPosition_Tank(int startRow, int startCol)
{
	for (int i = startRow; i <= startRow + 4; i++)
	{
		for (int j = startCol; j <= startCol + 4; j++)
		{
			if (gameMap[i][j] != ' ')
				return false;
		}
	}
	return true;
}

// Randomly spawn a tank on the map
void spawn_Tank(int startRow, int startCol, int xB_t, int yB_t)
{
	int max_Tanks;      // Maximum number of tanks in different levels

	if (LEVEL == 1)
		max_Tanks = 3;  // There are maximum of three tanks in level_1
	else
		max_Tanks = 5;  // There are maximum of five tanks in level_2

	if (NO_TANKS < max_Tanks)
	{
		srand(time(0));
		do
		{
			startRow = 1;
			startCol = rand() % (WIDTH - 6) + 1;
		} while (!validPosition_Tank(startRow, startCol));

		create_Tank(startRow, startCol);

		for (int i = 0; i < 5; i++)
		{
			if (i == NO_TANKS)
			{
				xB_t = startCol + 2; // Position of Bullet according to column
				yB_t = startRow + 4; // Position of Bullet according to row
				gameMap[yB_t][xB_t] = 'o';
			}
		}
		NO_TANKS++; // When tank appears its number increases by one
	}

}

// Spawn tank again if it has less in numbers as allowed
void spawn_Tank_Again()
{
	int spawnObsCrAgain = rand() % 8; // Tank spawn again when previous one diminishes from map
	if (spawnObsCrAgain == 0)
		spawn_Tank(0, 0, 0, 0);
}

// Moves the bullet down considering the map layout
void extra_Movements()
{
	bool foundBullet = false;
	bool foundRambo = false;

	// Loop through the map to find the bullet position
	for (int i = 1; i < HEIGHT - 1; i++)
	{
		for (int j = 1; j < WIDTH; j++)
		{
			if (gameMap[i][j] == 'o') // If bullet found at this position
			{
				gameMap[i][j] = ' ';  // Fill the previous bullet position with space

				if (gameMap[i + 1][j] == ' ') // Check if the new position for the bullet is empty
					gameMap[i + 1][j] = 'o';  // Set the bullet to new position

				foundBullet = true;
			}
			if (gameMap[i][j] == '_' && i != HEIGHT - 2)
			{
				gameMap[i][j] = ' ';
				gameMap[i + 1][j - 1] = ' ';
				gameMap[i + 1][j + 1] = ' ';
				if (gameMap[i][j] == ' ' && gameMap[i][j - 1] == ' ' && gameMap[i][j + 1] == ' ')
				{
					gameMap[i][j] = '_';
					gameMap[i][j - 1] = '^';
					gameMap[i][j + 1] = '^';
				}
				foundRambo = true;

			}
		}
		if (foundBullet)
			i++; // Skip the next row because bullet is already went this row
		if (foundRambo)
			i = i; // Skip the next row
	}
}
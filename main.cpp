#include <iostream>
#include <string>
#include <stdlib.h>
#include <cctype>
#include <cmath>
#include <unistd.h>
#include <termios.h>
#include "images.cpp"
using namespace std;


struct Plate
{
	int position;
	bool isTriggered = false;
};

struct Items
{
	bool key = false;
};

char getch();
void moveToNextPos(string [], int, int, int);
bool gamePhysics(string[], char, int &, int, int, int, int, Items &);
void keyToContinue();
void setPlates(string [], Plate [], int);
void displayArea(string [], int);
void blockOffPlate(string [], Plate [], int, int &, int, int);
bool moveCharacter(char, Items &, string[], int, int &, int, Plate[], int, string);

void moveToNextPos(string area[], int playerPos, int directionAdd, int addOrSub)
{
	int storage = playerPos;
	if(addOrSub == 1)
	{
		area[playerPos + directionAdd] = PLAYER_IMG;
		playerPos += directionAdd;
		area[storage] = OPEN_SPACE;
	}
	else
	{
		area[playerPos - directionAdd] = PLAYER_IMG;
		playerPos -= directionAdd;
		area[storage] = OPEN_SPACE;
	}
}

char getch()
{
    char buf = 0;
    struct termios old = {0}; // Makes a termios structure.
    if (tcgetattr(0, &old) < 0) // If the terminal fails:
            perror("tcsetattr()"); // Display an error message.
	// c_lflag 
    old.c_lflag &= ~ICANON; // Set the terminal into noncononical mode.
    old.c_lflag &= ~ECHO; // Disable echo in the terminal.
    old.c_cc[VMIN] = 1; // Sets the character limit to one in the terminal.
    old.c_cc[VTIME] = 0; // Sets no time limit in the terminal.
    if (tcsetattr(0, TCSANOW, &old) < 0) // If there's an error when setting the attributes: 
            perror("tcsetattr ICANON"); // Displays an error message.
    if (read(0, &buf, 1) < 0) // If there is an error reading the terminal's data and assigning it to buf: 
            perror ("read()"); // Display an error message.
    old.c_lflag |= ICANON; // Revert the terminal back into cononical mode.
    old.c_lflag |= ECHO; // Re-enable echo.
    if (tcsetattr(0, TCSADRAIN, &old) < 0) // If the attributes failed to set:
            perror ("tcsetattr ~ICANON"); // Display an error message.
    return (buf); // Return the character entered by the user. 
}

void keyToContinue()
{
	cout << "Type any key to continue..." << endl;
	getch();
	return;
}

bool gamePhysics(string area[], char keyToPress, int & playerPos, int directionAdd, int areaSize, int endPos, string endOfGameMsg, Items & items, Plate plates[], int numOfPlates)
{
	int storage = 0;
	int sqrtAreaSize = sqrt(areaSize);
	bool allPlatesPressed = false;
	int platePressedCounter = 0;

	if(keyToPress == 'W')
	{
		if(playerPos - sqrtAreaSize < 0)
		return false;
	}

	for(int i = 0; i < numOfPlates; i++)
	{
		if(plates[i].isTriggered)
		{
			platePressedCounter++;
		}
	}
	
	if(platePressedCounter == numOfPlates)
	{
		allPlatesPressed = true;
	}
	else allPlatesPressed = false;

	
	// Going backwards in the array, therefore uses subtraction.
	if(keyToPress == 'W' or keyToPress == 'A')
	{
		if(area[playerPos - directionAdd] == TRIGGERED_PLATE and area[playerPos - directionAdd - directionAdd] == VERTICAL_WALL or area[playerPos - directionAdd] == TRIGGERED_PLATE and area[playerPos - directionAdd - directionAdd] == HORIZONTAL_WALL)
		{ // If the position the player is going to is a triggered plate and the spot after the triggered plate is a wall or another triggered plate, they can't move.
			return false;
		}
		if(area[playerPos - directionAdd] != VERTICAL_WALL and area[playerPos - directionAdd] != HORIZONTAL_WALL)
		{ // Checks if the player is not moving into a wall.
			if(area[playerPos - directionAdd] == LOCK and items.key == false or area[playerPos - directionAdd] == LOCK and allPlatesPressed == false)
			{ // Checks if the player is moving into a lock without all the plates being pressed or without the key.
				system("clear");
				return false;
			}
			else if(area[playerPos - directionAdd] == KEY)
			{ // If the player moves into the key, they get the key. 
				items.key = true;
			}
			else if(area[playerPos - directionAdd] == BLOCK)
			{ // Checks if the player walks into a block.
				if(area[playerPos - directionAdd - directionAdd] == OPEN_SPACE or area[playerPos - directionAdd - directionAdd] == PLATE)
				{ // Checks if the block can move the direction the player is moving.
					storage = playerPos;
					area[playerPos - directionAdd] = PLAYER_IMG;
					area[playerPos - directionAdd - directionAdd] = BLOCK;
					playerPos -= directionAdd; // ======================= Potential optimization. Function
					area[storage] = OPEN_SPACE; 
					system("clear");
				}
				return false;
			}
			else if(area[playerPos - directionAdd] == TRIGGERED_PLATE and area[playerPos - directionAdd - directionAdd] != TRIGGERED_PLATE and area[playerPos - directionAdd - directionAdd] != VERTICAL_WALL and area[playerPos - directionAdd - directionAdd] != HORIZONTAL_WALL)
			{ // Checks if you can take the block out of the triggered plate by checking if there is another triggered plate or a wall after it. 
				blockOffPlate(area, plates, numOfPlates, playerPos, directionAdd, -1);
			}
			else if(area[playerPos - directionAdd] == TRIGGERED_PLATE and area[playerPos - directionAdd - directionAdd] == TRIGGERED_PLATE)
			{
				return false;
			}
			else if(area[playerPos - directionAdd] == ICE)
			{ // Checks if the player is walking onto ice. 
				while(area[playerPos - directionAdd] == ICE)
				{ // While the player is on ice, move the player onto the mext piece of ice.
					storage = playerPos;
					area[playerPos - directionAdd] = PLAYER_IMG;
					playerPos -= directionAdd;
					
					if(area[storage] == ICE)
					{
						area[storage] = ICE;
					}
					else area[storage] = OPEN_SPACE; // If the player came from a position not on ice, set that position to open space.
					
					system("clear");
					displayArea(area, areaSize);
					if(area[playerPos - directionAdd] == TRIGGERED_PLATE and area[playerPos - directionAdd - directionAdd] != TRIGGERED_PLATE and area[playerPos - directionAdd - directionAdd] != VERTICAL_WALL and area[playerPos - directionAdd - directionAdd] != HORIZONTAL_WALL)
					{ // Checks if the player ends on a triggered plate.
						blockOffPlate(area, plates, numOfPlates, playerPos, directionAdd, -1);
					}
				}
			}
			storage = playerPos;
			area[playerPos - directionAdd] = PLAYER_IMG;
			playerPos -= directionAdd;
			area[storage] = OPEN_SPACE;
			system("clear");
			if(playerPos == endPos)
			{
				cout << endOfGameMsg << endl;
				keyToContinue();
				return true;
			}
		}
	}	
	else // Going backwards in the array (S and D), therefore uses addition.
	{
		if(area[playerPos + directionAdd] != VERTICAL_WALL and area[playerPos + directionAdd] != HORIZONTAL_WALL)
		{
			if(area[playerPos + directionAdd] == TRIGGERED_PLATE and area[playerPos + directionAdd + directionAdd] == VERTICAL_WALL or area[playerPos + directionAdd] == TRIGGERED_PLATE and area[playerPos + directionAdd + directionAdd] == HORIZONTAL_WALL)
			{
				return false;
			}
			if(area[playerPos + directionAdd] == LOCK and items.key == false or area[playerPos + directionAdd] == LOCK and allPlatesPressed == false)
			{
				return false;
			}
			else if(area[playerPos + directionAdd] == KEY)
			{
				items.key = true;
			}
			else if(area[playerPos + directionAdd] == BLOCK)
			{
				if(area[playerPos + directionAdd + directionAdd] == OPEN_SPACE or area[playerPos + directionAdd + directionAdd] == PLATE)
				{
					storage = playerPos;
					area[playerPos + directionAdd] = PLAYER_IMG;
					area[playerPos + directionAdd + directionAdd] = BLOCK;
					playerPos += directionAdd;
					area[storage] = OPEN_SPACE;
					system("clear");
				}
				else
				{
					system("clear");
				}
				return false;
			}
			else if(area[playerPos + directionAdd] == TRIGGERED_PLATE and area[playerPos + directionAdd + directionAdd] != TRIGGERED_PLATE and area[playerPos + directionAdd + directionAdd] != VERTICAL_WALL and area[playerPos + directionAdd + directionAdd] != HORIZONTAL_WALL)
			{
				blockOffPlate(area, plates, numOfPlates, playerPos, directionAdd, 1);
			}
			else if(area[playerPos + directionAdd] == TRIGGERED_PLATE and area[playerPos + directionAdd + directionAdd] == TRIGGERED_PLATE)
			{
				return false;
			}
			else if(area[playerPos + directionAdd] == ICE)
			{
				while(area[playerPos + directionAdd] == ICE)
				{
					storage = playerPos;
					area[playerPos + directionAdd] = PLAYER_IMG;
					playerPos += directionAdd;
					if(area[storage] == ICE)
					{
						area[storage] = ICE;		
					}
					else area[storage] = OPEN_SPACE;
					
					system("clear");
					displayArea(area, areaSize);
					if(area[playerPos + directionAdd] == TRIGGERED_PLATE and area[playerPos + directionAdd + directionAdd] != TRIGGERED_PLATE and area[playerPos + directionAdd + directionAdd] != VERTICAL_WALL and area[playerPos + directionAdd + directionAdd] != HORIZONTAL_WALL)
					{
						blockOffPlate(area, plates, numOfPlates, playerPos, directionAdd, 1);
					}
				}
			}
			
			storage = playerPos;
			area[playerPos + directionAdd] = PLAYER_IMG;
			playerPos += directionAdd;
			area[storage] = OPEN_SPACE;
			
			system("clear");
			if(playerPos == endPos)
			{
				displayArea(area, areaSize);
				cout << endOfGameMsg << endl;
				keyToContinue();
				return true;
			}
		}
	}
	return false;
}

/// ===============================================

void setPlates(string mainArea[], Plate plates[], int numOfPlates)
{
	for(int i = 0; i < numOfPlates; i++)
	{
		if(mainArea[plates[i].position] != PLAYER_IMG and mainArea[plates[i].position] != BLOCK and plates[i].isTriggered == false)
		{
			mainArea[plates[i].position] = PLATE;
		}
		else if(mainArea[plates[i].position] == BLOCK or mainArea[plates[i].position] == TRIGGERED_PLATE or plates[i].isTriggered == true and mainArea[plates[i].position] != PLAYER_IMG)
		{
			mainArea[plates[i].position] = TRIGGERED_PLATE;
			plates[i].isTriggered = true;
		}
	}
	return;
}

/// ===============================================

void blockOffPlate(string area[], Plate plates[], int numOfPlates, int & playerPos, int directionAdd, int addOrSubtract)
{
	if(addOrSubtract == -1)
	{
		for(int i = 0; i < numOfPlates; i++)
		{
			if(plates[i].position == (playerPos - directionAdd))
			{
				plates[i].isTriggered = false;
			}
		}
		area[playerPos - directionAdd - directionAdd] = BLOCK; 
	}
	else if(addOrSubtract == 1)
	{
		for(int i = 0; i < numOfPlates; i++)
		{
			if(plates[i].position == (playerPos + directionAdd))
			{
				plates[i].isTriggered = false;
			}
		}
		area[playerPos + directionAdd + directionAdd] = BLOCK; 
	}
}

// ================================================

void displayArea(string area[], int size)
{
	int sqrtSize = sqrt(size);
	for (int i = 0; i < size; i++)
	{
		cout << area[i];
		if(area[i+1] != LOCK)
		{
			cout << " ";
		}
		
		if((i+1)%sqrtSize == 0)
		{
			cout << endl;
		}
	}
	return;
}


/// =================================================

void setIce(string area[], int _icePositions[], int numOfIce)
{
	for(int i = 0; i < numOfIce; i++)
	{
		if(area[_icePositions[i]] != PLAYER_IMG and area[_icePositions[i]] != BLOCK)
		{
			area[_icePositions[i]] = ICE;
		}
	}
	return;
}

/// =================================================

bool moveCharacter(char _playerDirection, Items & items, string area[], int areaSize, int & playerPos, int endPos, Plate plates[], int numOfPlates, string endOfGameMsg)
{
	bool gameComplete = false;
	int sqrtAreaSize = sqrt(areaSize);
	if(_playerDirection == 'W' or _playerDirection == 'S')
	{
		gameComplete = gamePhysics(area, _playerDirection, playerPos, sqrtAreaSize, areaSize, endPos, endOfGameMsg, items, plates, numOfPlates);
	}
	else if(_playerDirection == 'A' or _playerDirection == 'D')
	{
		gameComplete = gamePhysics(area, _playerDirection, playerPos, 1, areaSize, endPos, endOfGameMsg, items, plates, numOfPlates);
	}
	return gameComplete;
}

/// =================================================

int main() 
{
	Plate plate1;
	Plate plate2;
	Plate plate3;
	plate1.position = 34;
	plate2.position = 71;
	plate3.position = 51;
	char playerDirection;
	int playerPos = 1;
	bool gameComplete = false;
	int areaSize = 100;
	int endPos = 91;
	int sqrtAreaSize = sqrt(areaSize);
	Plate plates[3] = {plate1, plate2, plate3};
	int icePositions[5] = {73, 74, 75, 76, 77};
	int numOfIce = 5;
	int numOfPlates = 3;
	Items items;

	string mainArea[100] = {
	VERTICAL_WALL, PLAYER_IMG, HORIZONTAL_WALL, HORIZONTAL_WALL, HORIZONTAL_WALL, HORIZONTAL_WALL, HORIZONTAL_WALL, HORIZONTAL_WALL, HORIZONTAL_WALL, VERTICAL_WALL,
	VERTICAL_WALL, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, VERTICAL_WALL,
	VERTICAL_WALL, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, VERTICAL_WALL,
	VERTICAL_WALL, OPEN_SPACE, BLOCK, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, VERTICAL_WALL,
	VERTICAL_WALL, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, VERTICAL_WALL,
	VERTICAL_WALL, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, BLOCK, OPEN_SPACE, OPEN_SPACE, VERTICAL_WALL,
	VERTICAL_WALL, OPEN_SPACE, BLOCK, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, VERTICAL_WALL,
	VERTICAL_WALL, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, VERTICAL_WALL,
	VERTICAL_WALL, OPEN_SPACE, OPEN_SPACE, KEY, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, OPEN_SPACE, VERTICAL_WALL,
	VERTICAL_WALL, LOCK, HORIZONTAL_WALL, HORIZONTAL_WALL, HORIZONTAL_WALL, HORIZONTAL_WALL, HORIZONTAL_WALL, HORIZONTAL_WALL, HORIZONTAL_WALL, VERTICAL_WALL
	};

	do
	{
		system("clear");
		setPlates(mainArea, plates, numOfPlates);
		setIce(mainArea, icePositions, numOfIce);
		displayArea(mainArea, areaSize);
		playerDirection = getch();
		playerDirection = toupper(playerDirection);
		gameComplete = moveCharacter(playerDirection, items, mainArea, areaSize, playerPos, endPos, plates, numOfPlates, "You made it!!!");

	}while(gameComplete == false);
	return 0;
}

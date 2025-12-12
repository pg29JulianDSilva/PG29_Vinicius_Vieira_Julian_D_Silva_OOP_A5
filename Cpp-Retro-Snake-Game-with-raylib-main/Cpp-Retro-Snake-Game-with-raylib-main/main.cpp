#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>
#include <vector>
#include <memory>

using namespace std;

static bool allowMove = false;
Color black = {0, 0, 0, 255};
Color darkGreen = {0, 255, 100, 255};
Color yellow = {255, 255, 0 , 255};
Color red = {255, 0, 0 , 255};
Color lightBlue = {0, 162, 232, 255};
Color blue = { 0, 194, 0, 255};

// The size of every tile
int cellSize = 30;
// The amount of tiles
int cellCount = 25;
// The margin offset of the screen
int offset = 75;

double lastUpdateTime = 0;

// Global functions

// Checks if there is a Vector2 inside a list, returns true if it finds one, false if not
bool ElementInDeque(Vector2 element, deque<Vector2> deque)
{
    for (unsigned int i = 0; i < deque.size(); i++)
    {
        if (Vector2Equals(deque[i], element))
        {
            return true;
        }
    }
    return false;
}

//class Player;
// 
bool EventTriggered(double interval)
{
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval)
    {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}


// class Ability
// {
// public:
    
    // virtual void ExecuteAbility();

    // virtual void initAbility(std::weak_ptr<Player> player)
    // {
    //     mOwningPlayer = player;
    // }

// protected:

//     std::weak_ptr<Player> mOwningPlayer = nullptr;
// };

// class SpeedUpAbility : public Ability
// {
//     public:

//     virtual void ExecuteAbility() override
//     {
//         if(std::shared_ptr<Player> PlayerToControl = mOwningPlayer.lock())
//         {
//             PlayerToControl->
//         }
//     }
// }

class PowerUp;

// List of different Power Ups
enum PowerUpType
{
    SPEED_BOOST,
    INVINCIBILITY
};

class Player
{
public:
    //Changed to two vectors for game over
    deque<Vector2> body = {};
    deque<Vector2> sBody;
    Vector2 direction = {1, 0};
    bool addSegment = false;
    bool isScrambled = false;
    double scrambledEnd = 0;
    double speedBoostEnd = 0;
    double invincibilityEnd = 0;
    PowerUpType powerType;

    // std::vector<std::shared_ptr<Ability>> mAbilities;
    // void AddAbility(std::shared_ptr<Ability> AbilityToAdd)
    // {
    //     mAbilities.emplace_back(AbilityToAdd);
    //     AbilityToAdd->initAbility(this);
    //     AbilityToAdd->ExecuteAbility();
    // }

    //Added for player selections
    Color thisColor;

    //NEW constructor, takes the color of the player and the initial position of the player
    Player(Color theColor, int Starting_Point){
        thisColor = theColor;
        
        for(int i = 2; i < 10; i++){
            body.push_front(Vector2Add(Vector2{i, Starting_Point}, direction));
            sBody = body;
        }
    }

    void Draw()
    {
        for (unsigned int i = 0; i < body.size(); i++)
        {
            float x = body[i].x;
            float y = body[i].y;
            //Reduced size
            Rectangle segment = {offset + x * cellSize, offset + y * cellSize, (float)cellSize/2, (float)cellSize/2};
            /*if(direction == Vector2{1,0}){
                segment = {offset + x * cellSize, offset + y * cellSize, (float)cellSize, (float)cellSize/2};
            }else{
                segment = {offset + x * cellSize, offset + y * cellSize, (float)cellSize/2, (float)cellSize};
            } It will be cool if we can implement this*/
            DrawRectangleRounded(segment, 0.5, 6, thisColor);
        }
    }

    void Update()
    {
        body.push_front(Vector2Add(body[0], direction));
        if (addSegment == true)
        {
            addSegment = false;
        }
        else
        {
            body.pop_back();
        }
    }

    void Reset()
    {

        speedBoostEnd = 0;
        invincibilityEnd = 0;
        body = sBody;
        direction = {1, 0};


    }

    void ApplyPowerUp(PowerUpType type)
    {
        double currentTime = GetTime();

        if (type == SPEED_BOOST)
        {
            scrambledEnd = currentTime + 5.0;
        }
        else if (type == INVINCIBILITY)
        {
            invincibilityEnd = currentTime + 5.0;
        }
    }

    bool IsScrammbled(Player player)
    {
        return GetTime() < this->scrambledEnd;
    }

    bool IsInvincible(Player player)
    {
        return GetTime() < this->invincibilityEnd;
    }
};



// Class for powerups
class PowerUp
{
public:
    Vector2 position;
    Texture2D texture;
    PowerUpType powerType;
    double spawnCooldown;

    // Constructor loading the correct image depending on the
    // selected power up to spawn
    PowerUp(deque<Vector2> snakeBody, deque<Vector2> snakeBody2, PowerUpType power)
    {
        powerType = power;
        spawnCooldown = GetTime();
        std::string path = "Graphics/";
        if (powerType == SPEED_BOOST)
        {
            path += "speed.png";
        }
        else if ( powerType == INVINCIBILITY)
        {
            path += "invincibility.png";
        }
        Image image = LoadImage(path.c_str());
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        position = GenerateRandomPos(snakeBody, snakeBody2);
    }

    ~PowerUp()
    {
        UnloadTexture(texture);
    }

    void Draw()
    {
        DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
    }

    Vector2 GenerateRandomCell()
    {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x, y};
    }

    Vector2 GenerateRandomPos(deque<Vector2> snakeBodyOne, deque<Vector2> snakeBodyTwo)
    {
        Vector2 position = GenerateRandomCell();
        while (ElementInDeque(position, snakeBodyOne) && ElementInDeque(position, snakeBodyTwo))
        {
            position = GenerateRandomCell();
        }
        return position;
    }

    //CHANGED FOR MULTIPLAYER
    Vector2 GenerateRandomPosAvoidingFoods(deque<Vector2> snakeBodyF, deque<Vector2> snakeBodyS, vector<PowerUp*> powers)
    {
        Vector2 position = GenerateRandomCell();
        while (ElementInDeque(position, snakeBodyF) && ElementInDeque(position, snakeBodyS))
        {
            position = GenerateRandomCell();
        }
        for (unsigned int i = 0; i < powers.size(); i++)
        {
            while (Vector2Equals(position, powers[i]->position))
            {
                position = GenerateRandomCell();
            }
        }
        return position;
    }
};

class Game
{
public:
    Player playerOne = Player(red, 9);
    Player playerTwo = Player(blue, 10);
    vector<PowerUp*> powerUps;
    bool running = true;
    int score = 0;
    Sound eatSound;
    Sound wallSound;
    int maxPowerups = 3;
    

    Game()
    {
        InitAudioDevice();
        eatSound = LoadSound("Sounds/eat.mp3");
        wallSound = LoadSound("Sounds/wall.mp3");

        // Spawn initial powerups
        for (int i = 0; i < maxPowerups; i++)
        {
            PowerUpType randomPowerType = (PowerUpType)GetRandomValue(0, 1);
            PowerUp* newPowerUp = new PowerUp(playerOne.body, playerTwo.body, randomPowerType);
            powerUps.push_back(newPowerUp);
        }
    }

    ~Game()
    {
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();
        for (unsigned int i = 0; i < powerUps.size(); i++)
        {
            delete powerUps[i];
        }

    }

    void Draw()
    {
        for (unsigned int i = 0; i < powerUps.size(); i++)
        {
            powerUps[i]->Draw();
        }
        playerOne.Draw();
        playerTwo.Draw();
    }

    void Update()
    {
        if (running)
        {
            playerOne.Update();
            playerTwo.Update();
            CheckCollisionWithPowerUps();
            CheckCollisionWithEdges();
            CheckCollisionWithTail();

            maxPowerups = 3 + (score / 10);

            while (powerUps.size() < maxPowerups)
            {
                PowerUpType randomPowerType = (PowerUpType)GetRandomValue(0, 1);
                PowerUp* newPowerUp = new PowerUp(playerOne.body, playerTwo.body, randomPowerType);
                powerUps.push_back(newPowerUp);
            }

            while (powerUps.size() > maxPowerups)
            {
                delete powerUps.back();
                powerUps.pop_back();
            }
        }
    }

    void CheckCollisionWithPowerUps()
    {
        for (unsigned int i = 0; i < powerUps.size(); i++)
        {
            if (Vector2Equals(playerOne.body[0], powerUps[i]->position))
            {
                playerOne.ApplyPowerUp(powerUps[i]->powerType);
                PowerUpType randomPowerType = (PowerUpType)GetRandomValue(0,1);
                delete powerUps[i];
                PowerUp* newPowerUp = new PowerUp(playerOne.body, playerTwo.body, randomPowerType);
                powerUps[i] = newPowerUp;
                score++;
                PlaySound(eatSound);
            }
            if (Vector2Equals(playerTwo.body[0], powerUps[i]->position))
            {
                playerTwo.ApplyPowerUp(powerUps[i]->powerType);
                PowerUpType randomPowerType = (PowerUpType)GetRandomValue(0,1);
                delete powerUps[i];
                PowerUp* newPowerUp = new PowerUp(playerOne.body, playerTwo.body, randomPowerType);
                powerUps[i] = newPowerUp;
                score++;
                PlaySound(eatSound);
            }
        }
    }

    void CheckCollisionWithEdges()
    {
        //Player 1
        if (playerOne.body[0].x == cellCount || playerOne.body[0].x == -1)
        {
            GameOver();
        }
        if (playerOne.body[0].y == cellCount || playerOne.body[0].y == -1)
        {
            GameOver();
        }

        //Player 2
        if (playerTwo.body[0].x == cellCount || playerTwo.body[0].x == -1)
        {
            GameOver();
        }
        if (playerTwo.body[0].y == cellCount || playerTwo.body[0].y == -1)
        {
            GameOver();
        }



    }

    void GameOver()
    {
        playerOne.Reset();
        playerTwo.Reset();
        for (unsigned int i = 0; i < powerUps.size(); i++)
        {
            powerUps[i]->position = powerUps[i]->GenerateRandomPosAvoidingFoods(playerOne.body, playerTwo.body, powerUps);
        }
        running = false;
        score = 0;
        maxPowerups = 3;
        PlaySound(wallSound);
    }

    void CheckCollisionWithTail()
    {
        // if (IsInvincible() == false)
        // {
            
        // }
        deque<Vector2> headlessBody1 = playerOne.body;
        deque<Vector2> headlessBody2 = playerTwo.body;
        headlessBody1.pop_front();
        headlessBody2.pop_front();

        //Added game over of second player into the check

        if(playerOne.IsInvincible(playerOne) == false)
        {
            if (ElementInDeque(playerOne.body[0], headlessBody1) || ElementInDeque(playerOne.body[0], headlessBody2))
            {
                GameOver();
            }
        }

        if(playerTwo.IsInvincible(playerTwo) == false)
        {
            if (ElementInDeque(playerTwo.body[0], headlessBody2) || ElementInDeque(playerTwo.body[0], headlessBody1))
            {
                GameOver();
            }
        }

    }
};

int main()
{
    cout << "Starting the game..." << endl;
    InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Retro TRON");
    SetTargetFPS(60);

    Game game = Game();

    while (WindowShouldClose() == false)
    {
        BeginDrawing();

        // Sets the movemenet speed based if its speed boosted or not
        // the numbers are the interval, so the smaller the number, the faster the player moves
        //double moveInterval = game.IsSpeedBoosted() ? 0.1 : 0.2;

        if (EventTriggered(0.1))
        {
            allowMove = true;
            game.Update();
        }

        // Player 1

        if (game.playerOne.IsScrammbled(game.playerOne))
        {
            if (IsKeyPressed(KEY_DOWN) && game.playerOne.direction.y != 1 && allowMove)
            {
                game.playerOne.direction = {0, -1};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_UP) && game.playerOne.direction.y != -1 && allowMove)
            {
                game.playerOne.direction = {0, 1};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_RIGHT) && game.playerOne.direction.x != 1 && allowMove)
            {
                game.playerOne.direction = {-1, 0};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_LEFT) && game.playerOne.direction.x != -1 && allowMove)
            {
                game.playerOne.direction = {1, 0};
                game.running = true;
                allowMove = false;
            }
        }
        else
        {
            if (IsKeyPressed(KEY_UP) && game.playerOne.direction.y != 1 && allowMove)
            {
                game.playerOne.direction = {0, -1};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_DOWN) && game.playerOne.direction.y != -1 && allowMove)
            {
                game.playerOne.direction = {0, 1};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_LEFT) && game.playerOne.direction.x != 1 && allowMove)
            {
                game.playerOne.direction = {-1, 0};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_RIGHT) && game.playerOne.direction.x != -1 && allowMove)
            {
                game.playerOne.direction = {1, 0};
                game.running = true;
                allowMove = false;
            }
        }
        

        //Player 2
        if (game.playerTwo.IsScrammbled(game.playerTwo))
        {
            if (IsKeyPressed(KEY_S) && game.playerTwo.direction.y != 1 && allowMove)
            {
                game.playerTwo.direction = {0, -1};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_W) && game.playerTwo.direction.y != -1 && allowMove)
            {
                game.playerTwo.direction = {0, 1};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_D) && game.playerTwo.direction.x != 1 && allowMove)
            {
                game.playerTwo.direction = {-1, 0};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_A) && game.playerTwo.direction.x != -1 && allowMove)
            {
                game.playerTwo.direction = {1, 0};
                game.running = true;
                allowMove = false;
            }
        }
        else
        {
            if (IsKeyPressed(KEY_W) && game.playerTwo.direction.y != 1 && allowMove)
            {
                game.playerTwo.direction = {0, -1};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_S) && game.playerTwo.direction.y != -1 && allowMove)
            {
                game.playerTwo.direction = {0, 1};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_A) && game.playerTwo.direction.x != 1 && allowMove)
            {
                game.playerTwo.direction = {-1, 0};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_D) && game.playerTwo.direction.x != -1 && allowMove)
            {
                game.playerTwo.direction = {1, 0};
                game.running = true;
                allowMove = false;
            }
        }
        

        // Drawing
        ClearBackground(black);
        DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10}, 5, darkGreen);
        DrawText("Retro TRON", offset - 5, 20, 40, darkGreen);
        DrawText(TextFormat("%i", game.score), offset - 5, offset + cellSize * cellCount + 10, 40, darkGreen);
        
        if (game.playerOne.IsScrammbled(game.playerOne) || game.playerTwo.IsScrammbled(game.playerTwo))
        {
            DrawText("Got Scrambled!", offset + 300, 20, 30, red);
        }
        if (game.playerOne.IsInvincible(game.playerOne)|| game.playerTwo.IsInvincible(game.playerTwo))
        {
            DrawText("Invincible!", offset + 600, 20, 30, lightBlue);
        }
        
        game.Draw();

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
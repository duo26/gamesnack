#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <iostream>
#include <vector>
#include <ctime>

using namespace std;

struct Point {
    int x, y;
};

enum GameState {
    START_SCREEN,
    PLAYING,
    GAME_OVER
};

class SnakeGame {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* backgroundTexture;
    SDL_Texture* startTexture;
    SDL_Texture* gameOverTexture;
    TTF_Font* font;
    SDL_Color textColor;
    SDL_Texture* scoreTexture;
    SDL_Texture* yourScoreTexture;
    int score;
    vector<Point> snake;
    Point fruit;
    int gridSize;
    int direction;
    bool running;
    GameState gameState;
    Mix_Music* backgroundMusic;
    Mix_Chunk* changeDirectionSound;
    Mix_Chunk* gameOverSound;

public:
    SnakeGame() {
        srand(time(0));
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
        IMG_Init(IMG_INIT_PNG);
        TTF_Init();
        Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
        window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        gridSize = 20;
        direction = 1;
        running = true;
        gameState = START_SCREEN;
        score = 0;
        textColor = {255, 255, 255, 255};
        snake.push_back({10, 10});
        spawnFruit();

        SDL_Surface* backgroundSurface = IMG_Load("background.jpg");
        if (!backgroundSurface) {
            cout << "Failed to load background image: " << IMG_GetError() << endl;
        } else {
            backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
            SDL_FreeSurface(backgroundSurface);
        }

        SDL_Surface* startSurface = IMG_Load("start.png");
        if (!startSurface) {
            cout << "Failed to load start screen image: " << IMG_GetError() << endl;
        } else {
            startTexture = SDL_CreateTextureFromSurface(renderer, startSurface);
            SDL_FreeSurface(startSurface);
        }
        SDL_Surface* gameOverSurface = IMG_Load("gameover.png");
        if (!gameOverSurface) {
            cout << "Failed to load game over image: " << IMG_GetError() << endl;
        } else {
            gameOverTexture = SDL_CreateTextureFromSurface(renderer, gameOverSurface);
            SDL_FreeSurface(gameOverSurface);
        }
        font = TTF_OpenFont("ARIALBD1.ttf", 24);
        if (!font) {
            cout << "Failed to load font: " << TTF_GetError() << endl;
        }

        backgroundMusic = Mix_LoadMUS("music.mp3");
        if (!backgroundMusic) {
            cout << "Failed to load background music: " << Mix_GetError() << endl;
        } else {
            Mix_PlayMusic(backgroundMusic, -1);
        }

        changeDirectionSound = Mix_LoadWAV("move.mp3");
        if (!changeDirectionSound) {
            cout << "Failed to load change direction sound: " << Mix_GetError() << endl;
        }

        gameOverSound = Mix_LoadWAV("gameover.mp3");
        if (!gameOverSound) {
            cout << "Failed to load game over sound: " << Mix_GetError() << endl;
        }
    }

    ~SnakeGame() {
        SDL_DestroyTexture(backgroundTexture);
        SDL_DestroyTexture(startTexture);
        SDL_DestroyTexture(gameOverTexture);
        SDL_DestroyTexture(scoreTexture);
        SDL_DestroyTexture(yourScoreTexture);
        Mix_FreeMusic(backgroundMusic);
        Mix_FreeChunk(changeDirectionSound);
        Mix_FreeChunk(gameOverSound);
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        Mix_CloseAudio();
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
    }

    void spawnFruit() {
        fruit.x = rand() % (640 / gridSize);
        fruit.y = rand() % (480 / gridSize);
    }

    void handleInput() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (gameState == START_SCREEN || gameState == GAME_OVER) {
                    if (event.key.keysym.sym == SDLK_SPACE) {
                        resetGame();
                        gameState = PLAYING;
                    }
                } else {
                    switch (event.key.keysym.sym) {
                        case SDLK_UP:
                            if (direction != 2) {
                                direction = 0;
                                Mix_PlayChannel(-1, changeDirectionSound, 0);
                            }
                            break;
                        case SDLK_RIGHT:
                            if (direction != 3) {
                                direction = 1;
                                Mix_PlayChannel(-1, changeDirectionSound, 0);
                            }
                            break;
                        case SDLK_DOWN:
                            if (direction != 0) {
                                direction = 2;
                                Mix_PlayChannel(-1, changeDirectionSound, 0);
                            }
                            break;
                        case SDLK_LEFT:
                            if (direction != 1) {
                                direction = 3;
                                Mix_PlayChannel(-1, changeDirectionSound, 0);
                            }
                            break;
                    }
                }
            }
        }
    }
    void update() {
        if (gameState != PLAYING) return;

        Point newHead = snake.front();
        switch (direction) {
            case 0:
                newHead.y--;
                break;
            case 1:
                newHead.x++;
                break;
            case 2:
                newHead.y++;
                break;
            case 3:
                newHead.x--;
                break;
        }

        if (newHead.x < 0 || newHead.x >= (640 / gridSize) || newHead.y < 0 || newHead.y >= (480 / gridSize)) {
            gameState = GAME_OVER;
            Mix_PlayChannel(-1, gameOverSound, 0);
            return;
        }

        snake.insert(snake.begin(), newHead);

        if (newHead.x == fruit.x && newHead.y == fruit.y) {
            spawnFruit();
            score += 10;
            updateScoreTexture();
        } else {
            snake.pop_back();
        }

        for (size_t i = 1; i < snake.size(); ++i) {
            if (newHead.x == snake[i].x && newHead.y == snake[i].y) {
                gameState = GAME_OVER;
                Mix_PlayChannel(-1, gameOverSound, 0);
                break;
            }
        }
    }

    void render() {
        if (gameState == START_SCREEN) {
            SDL_RenderCopy(renderer, startTexture, NULL, NULL);
        } else if (gameState == GAME_OVER) {
            SDL_RenderCopy(renderer, gameOverTexture, NULL, NULL);
            renderYourScore();
        } else {
            SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            for (const auto& segment : snake) {
                SDL_Rect rect = {segment.x * gridSize, segment.y * gridSize, gridSize, gridSize};
                SDL_RenderFillRect(renderer, &rect);
            }

            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_Rect fruitRect = {fruit.x * gridSize, fruit.y * gridSize, gridSize, gridSize};
            SDL_RenderFillRect(renderer, &fruitRect);

            renderScore();
        }
        SDL_RenderPresent(renderer);
    }
    void updateScoreTexture() {
        SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, ("Score: " + to_string(score)).c_str(), textColor);
        scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
        SDL_FreeSurface(scoreSurface);
    }
    void renderScore() {
        if (scoreTexture) {
            int text_width = 0;
            int text_height = 0;
            SDL_QueryTexture(scoreTexture, NULL, NULL, &text_width, &text_height);
            SDL_Rect renderQuad = {10, 10, text_width, text_height};
            SDL_RenderCopy(renderer, scoreTexture, NULL, &renderQuad);
        }
    }
    void renderYourScore() {
        string message = "Your Score: " + to_string(score);
        SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, message.c_str(), textColor);
        yourScoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
        SDL_FreeSurface(scoreSurface);
        if (yourScoreTexture) {
            int text_width = 0;
            int text_height = 0;
            SDL_QueryTexture(yourScoreTexture, NULL, NULL, &text_width, &text_height);
            SDL_Rect renderQuad = {320 - text_width / 2, 240 - text_height / 2, text_width, text_height};
            SDL_RenderCopy(renderer, yourScoreTexture, NULL, &renderQuad);
        }
    }
    void resetGame() {
        score = 0;
        snake.clear();
        snake.push_back({10, 10});
        direction = 1;
        spawnFruit();
        updateScoreTexture();
    }
    void run() {
        while (running) {
            handleInput();
            update();
            render();
            SDL_Delay(100);
        }
    }
};
int main(int argc, char* argv[]) {
    SnakeGame game;
    game.run();
    return 0;
}

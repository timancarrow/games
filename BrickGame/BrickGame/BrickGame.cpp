/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>
#include <array>

//Screen dimension constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;

//Score Board dimension
const int SCOREBOARD_WIDTH = 800; 
const int SCOREBOARD_HEIGHT = 200; 

// Game globals
const int numBrickTypes = 6; 
const int numPaddleTypes = 6; 
const int numBallTypes = 2; 
const int numGameBricks = 36; 


//Brick sides enum
enum brickside
{
	NONE, TOP, RIGHT, BOTTOM, LEFT
};

//A circle stucture
struct Circle
{
	int x, y;
	int r;
};

//Texture wrapper class
class LTexture
{
	public:
		//Initializes variables
		LTexture();

		//Deallocates memory
		~LTexture();

		//Loads image at specified path
		bool loadFromFile( std::string path );
		
		#ifdef _SDL_TTF_H
		//Creates image from font string
		bool loadFromRenderedText( std::string textureText, SDL_Color textColor );
		#endif

		//Deallocates texture
		void free();

		//Set color modulation
		void setColor( Uint8 red, Uint8 green, Uint8 blue );

		//Set blending
		void setBlendMode( SDL_BlendMode blending );

		//Set alpha modulation
		void setAlpha( Uint8 alpha );
		
		//Renders texture at given point
		void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

		//Gets image dimensions
		int getWidth();
		int getHeight();

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;

		//Image dimensions
		int mWidth;
		int mHeight;
};

//The application time based timer
class LTimer
{
    public:
		//Initializes variables
		LTimer();

		//The various clock actions
		void start();
		void stop();
		void pause();
		void unpause();

		//Gets the timer's time
		Uint32 getTicks();

		//Checks the status of the timer
		bool isStarted();
		bool isPaused();

    private:
		//The clock time when the timer started
		Uint32 mStartTicks;

		//The ticks stored when the timer was paused
		Uint32 mPausedTicks;

		//The timer status
		bool mPaused;
		bool mStarted;
};

class paddle
{
    public:
		//paddle dimensions
		static const int paddle_width = 200;
		static const int paddle_height = 24;

		//Maximum axis velocity of the paddle
		static const int paddle_vel = 8;

		//Initializes the variables
		paddle();

		//Takes key presses and adjusts the paddles velocity
		void handleEvent( SDL_Event& e );

		//Moves the paddle
		void move();

		//Shows the brick on the screen
		void render();

		SDL_Rect mPaddleCollider;

		//The velocity of the paddle
		int mVelX, mVelY;

		//The X and Y offsets
		int mPosX, mPosY;

    private:
		
		void shiftColliders();
};

class brick
{
    public:
		//Brick dimensions
		static const int brick_width = 80;
		static const int brick_height = 20;
		bool hitbyball;
		brickside sidehit;
		int bricktype; 

		SDL_Rect brickRect;

		//Initializes the variables
		brick();

		//Shows the brick on the screen
		void render();

		//Arrange brick in correct position
		void arrange(int posX, int posY);

    private:
		//The X and Y offsets
		int mPosX, mPosY;
};

class ball
{
    public:
		//The dimensions of the ball
		static const int ball_WIDTH = 20;
		static const int ball_HEIGHT = 20;

		//Maximum axis velocity of the ball
		static const int ball_VEL = 6;

		//Initializes the variables
		ball();

		//Takes key presses and adjusts the ball
		void handleEvent( SDL_Event& e, bool gameOn);

		//Moves the ball
		void move(std::vector<brick> &gameBricks, paddle &gamePaddle);

		//Shows the ball on the screen
		void render();

		// ball collision circle
		Circle mBallCollider;

		//The X and Y offsets of the ball
		int mPosX, mPosY;

    private:
		//The velocity of the ball
		int mVelX, mVelY;

		////Moves the collision circle relative to the balls offset
		void shiftColliders();
};

class scoreboard 
{
public: 
	
	// variable to hold current fps
	double avgFPS; 

	// number of bricks cleared	
	int gamescore; 

	scoreboard(); 

	void render();
}; 

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//Circle/Box collision detector
bool checkCollision( Circle& a, SDL_Rect& b );

brickside checkCollisionSide(Circle& a, SDL_Rect& b);

bool updateCollisionSide(Circle& a, brick& b);

//Calculates distance squared between two points
double distanceSquared( int x1, int y1, int x2, int y2 );

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Scene textures
LTexture gDotTexture;
LTexture gBrickTexture;
LTexture gPaddleTexture;
LTexture gBallTexture;
LTexture gScoreBoardTexture; 
LTexture gScoreTextHeaderTexture; 
LTexture gFPSTextHeaderTexture;
LTexture gScoreTextTexture; 
LTexture gFPSTextTexture;
SDL_Color textColor = {41, 41, 41};

//Clips
SDL_Rect gPaddleClips[numPaddleTypes]; 
SDL_Rect gBrickClips[numBrickTypes];
SDL_Rect gBallClips[numBallTypes];
SDL_Rect gScoreBoardClip; 

//Global Font
TTF_Font *gFont = NULL; 


LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile( std::string path )
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL )
	{
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else
	{
		//Color key image
		SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );

		//Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

#ifdef _SDL_TTF_H
bool LTexture::loadFromRenderedText( std::string textureText, SDL_Color textColor )
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, textureText.c_str(), textColor );
	if( textSurface != NULL )
	{
		//Create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
		if( mTexture == NULL )
		{
			printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface( textSurface );
	}
	else
	{
		printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
	}

	
	//Return success
	return mTexture != NULL;
}
#endif

void LTexture::free()
{
	//Free texture if it exists
	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue )
{
	//Modulate texture rgb
	SDL_SetTextureColorMod( mTexture, red, green, blue );
}

void LTexture::setBlendMode( SDL_BlendMode blending )
{
	//Set blending function
	SDL_SetTextureBlendMode( mTexture, blending );
}
		
void LTexture::setAlpha( Uint8 alpha )
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx( gRenderer, mTexture, clip, &renderQuad, angle, center, flip );
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

LTimer::LTimer()
{
    //Initialize the variables
    mStartTicks = 0;
    mPausedTicks = 0;

    mPaused = false;
    mStarted = false;
}

void LTimer::start()
{
    //Start the timer
    mStarted = true;

    //Unpause the timer
    mPaused = false;

    //Get the current clock time
    mStartTicks = SDL_GetTicks();
	mPausedTicks = 0;
}

void LTimer::stop()
{
    //Stop the timer
    mStarted = false;

    //Unpause the timer
    mPaused = false;

	//Clear tick variables
	mStartTicks = 0;
	mPausedTicks = 0;
}

void LTimer::pause()
{
    //If the timer is running and isn't already paused
    if( mStarted && !mPaused )
    {
        //Pause the timer
        mPaused = true;

        //Calculate the paused ticks
        mPausedTicks = SDL_GetTicks() - mStartTicks;
		mStartTicks = 0;
    }
}

void LTimer::unpause()
{
    //If the timer is running and paused
    if( mStarted && mPaused )
    {
        //Unpause the timer
        mPaused = false;

        //Reset the starting ticks
        mStartTicks = SDL_GetTicks() - mPausedTicks;

        //Reset the paused ticks
        mPausedTicks = 0;
    }
}

Uint32 LTimer::getTicks()
{
	//The actual timer time
	Uint32 time = 0;

    //If the timer is running
    if( mStarted )
    {
        //If the timer is paused
        if( mPaused )
        {
            //Return the number of ticks when the timer was paused
            time = mPausedTicks;
        }
        else
        {
            //Return the current time minus the start time
            time = SDL_GetTicks() - mStartTicks;
        }
    }

    return time;
}

bool LTimer::isStarted()
{
	//Timer is running and paused or unpaused
    return mStarted;
}

bool LTimer::isPaused()
{
	//Timer is running and paused
    return mPaused && mStarted;
}

paddle::paddle()
{
    //Initialize the paddle at the bottom middle
	mPosX = SCREEN_WIDTH/2 - (paddle_width/2); // in the middle 
	mPosY = SCREEN_HEIGHT - SCOREBOARD_HEIGHT - paddle_height; // bottom of the screen minus the height of the paddle

    //Initialize the velocity
    mVelX = 0;
    mVelY = 0;

	mPaddleCollider.h = paddle_height;
	mPaddleCollider.w = paddle_width;
}

void paddle::handleEvent( SDL_Event& e )
{
    //If a key was pressed
	if( e.type == SDL_KEYDOWN && e.key.repeat == 0 )
    {
        //Adjust the velocity
        switch( e.key.keysym.sym )
        {
            //case SDLK_UP: mVelY -= paddle_vel; break;
            //case SDLK_DOWN: mVelY += paddle_vel; break;
            case SDLK_LEFT: mVelX -= paddle_vel; break;
            case SDLK_RIGHT: mVelX += paddle_vel; break;
        }
    }
    //If a key was released
    else if( e.type == SDL_KEYUP && e.key.repeat == 0 )
    {
        //Adjust the velocity
        switch( e.key.keysym.sym )
        {
           // case SDLK_UP: mVelY += paddle_vel; break;
           // case SDLK_DOWN: mVelY -= paddle_vel; break;
            case SDLK_LEFT: mVelX += paddle_vel; break;
            case SDLK_RIGHT: mVelX -= paddle_vel; break;
        }
    }
}

void paddle::move()
{
    //Move the paddle left or right
    mPosX += mVelX;
	shiftColliders();

    //If the paddle went too far to the left or right
    if( ( mPosX < 0 ) || ( mPosX + paddle_width> SCREEN_WIDTH ) )
    {
        //Move back
        mPosX -= mVelX;
    }
}

void paddle::render()
{
    //Show the paddle
		gPaddleTexture.render(mPosX, mPosY, &gPaddleClips[0]);
}

void paddle::shiftColliders() 
{
	mPaddleCollider.x = mPosX; 
	mPaddleCollider.y = mPosY; 
}

brick::brick()
{
	mPosX = 0; 
	mPosY = 0; 
	brickRect.x = 0; 
	brickRect.y = 0; 
	brickRect.h = 0; 
	brickRect.w = 0; 
	sidehit = NONE; 
	hitbyball = false;
	bricktype = 0; 
}

void brick::render()
{
	gBrickTexture.render(mPosX, mPosY, &gBrickClips[bricktype]);
}

void brick::arrange(int posX, int posY)
{
	mPosX = posX; 
	mPosY = posY;

	brickRect.x = posX; 
	brickRect.y = posY; 
	brickRect.w = brick_width;
	brickRect.h = brick_height;
}

ball::ball()
{
    //Initialize the offsets
	mPosX = -200;
	mPosY = -200; 

	// Set collision circle size
	mBallCollider.r = ball_WIDTH/2; 

    //Initialize the velocity
    mVelX = 0;
    mVelY = 0;

	//// move collider relative to the circle
	shiftColliders();
}

void ball::handleEvent( SDL_Event& e, bool gameOn )
{
    //If a key was pressed
	if( e.type == SDL_KEYDOWN && e.key.repeat == 0 && !gameOn )
    {
        //Adjust the velocity
        switch( e.key.keysym.sym )
        {
            case SDLK_SPACE: 
				{
					mVelY -= ball_VEL; 
					mVelX += ball_VEL;
					break;
				}

        }
    }
}

void ball::move(std::vector<brick> &gameBricks, paddle &gamePaddle)
{
    //Move the ball left or right
    mPosX += mVelX;
	shiftColliders();

	//Move the ball up or down
    mPosY += mVelY;
	shiftColliders(); 

    //Check left/right Screen Boundary collisions
	if( (mPosX - mBallCollider.r < 0) || (mPosX + mBallCollider.r > SCREEN_WIDTH))
	{
        //Move ball back and invert x velocity to make it bounce
        mPosX -= mVelX;
		mVelX = mVelX*-1;
		shiftColliders();
    }

	//Check up/down Screen Boundary collisions
	if( ( mPosY - mBallCollider.r < 0 ) || ( mPosY + mBallCollider.r > SCREEN_HEIGHT))
    {
        //Invert Y velocity to make it bounce
        mPosY -= mVelY;
		mVelY = -1*mVelY;
		shiftColliders();
    }
	
	//Check for a brick collision
	for(int c = 0; c < gameBricks.size(); c++)
	{
		/* +opt - an optimization can be made here. We are checking every brick for collision but technically a ball could not collide with
	       two bricks at the same time so we should break the evaluation whenever one collision occurs. */
		if(checkCollision(mBallCollider, gameBricks[c].brickRect))
		{
		//collision with a brick, mark the brick as hit and on which side
		//update the balls trajectory
			gameBricks[c].hitbyball = true;
			updateCollisionSide(mBallCollider, gameBricks[c]);

			switch (gameBricks[c].sidehit)
			{
				case TOP:
					{
						mPosY -= mVelY;
						mVelY = -1*mVelY;
						shiftColliders();
						break;
					}
				case BOTTOM:
					{
						mPosY -= mVelY;
						mVelY = -1*mVelY;
						shiftColliders();
						break;
					}

				case RIGHT:
					{
						mPosX -= mVelX;
						mVelX = mVelX*-1;
						shiftColliders();
						break;
					}
				case LEFT:
					{
						mPosX -= mVelX;
						mVelX = mVelX*-1;
						shiftColliders();
						break;
					}
			}
		}
	}

	//check for collision with paddle
	if(checkCollision(mBallCollider, gamePaddle.mPaddleCollider))
	{
		//change ball velocity based on paddles velocity
		mVelX = mVelX + gamePaddle.mVelX/4; 

		//invert y to bounce
		mPosY -= mVelY;
		mVelY = -1*mVelY;

		//update balls collider
		shiftColliders();

	}
}

void ball::render()
{
    //Show the ball
	gBallTexture.render( mPosX - mBallCollider.r, mPosY - mBallCollider.r, &gBallClips[0] );
}

void ball::shiftColliders()
{
	mBallCollider.x = mPosX; 
	mBallCollider.y = mPosY; 
}

scoreboard::scoreboard()
{
	int fps = 0; 
	int gamescore = 0; 
}

void scoreboard::render()
{
	gScoreBoardTexture.render(0, 0, &gScoreBoardClip); 
}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			printf( "Warning: Linear texture filtering not enabled!" );
		}

		//Create window
		gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 195, 195, 195, 0xFF);

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					success = false;
				}

				 //Initialize SDL_ttf
                if( TTF_Init() == -1 )
                {
                    printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
                    success = false;
                }
			}
		}
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Load paddle, brick, ball texture
	if( !gPaddleTexture.loadFromFile( "media/paddlesspritesheet.png" ) | !gBrickTexture.loadFromFile("media/bricksspritesheet.png")
		| !gBallTexture.loadFromFile("media/ballsspritesheet.png"))
	{
		printf( "Failed to load paddle, ball, brick sprite texture!\n" );
		success = false;
	}
	else
	{
		gPaddleClips[0].x = 0; 
		gPaddleClips[0].y = 0; 
		gPaddleClips[0].w = 200; 
		gPaddleClips[0].h = 24; 	


		for(int r = 0; r < numBrickTypes; r++)
		{
			gBrickClips[r].x = 0; 
			gBrickClips[r].y = r*20; 
			gBrickClips[r].w = 80;
			gBrickClips[r].h = 20; 

		}

		// Load Ball
		gBallClips[0].x = 0; 
		gBallClips[0].y = 0; 
		gBallClips[0].w = 20;
		gBallClips[0].h = 20; 
		
	}

	//load scoreboard textures
	if(!gScoreBoardTexture.loadFromFile( "media/scoreboard.png" ))
	{
		printf( "Failed to load score board texture!\n" );
		success = false;
	}
	else
	{
		gScoreBoardClip.x = 0; 
		gScoreBoardClip.y = 0; 
		gScoreBoardClip.h = SCOREBOARD_HEIGHT; 
		gScoreBoardClip.w = SCOREBOARD_WIDTH; 		
	}
	
	gFont = TTF_OpenFont( "media/alterebro.ttf", 28);  

	if(gFont == NULL)
	{
		printf( "Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError() );
        success = false;
	}
	else 
	{
		gScoreTextHeaderTexture.loadFromRenderedText("Score: ", textColor); 
		gFPSTextHeaderTexture.loadFromRenderedText("FPS: ", textColor); 
	}

	return success;
}

void close()
{
	//Free loaded images
	gDotTexture.free();
	gBallTexture.free();
	gPaddleTexture.free();

	//Free global font
	TTF_CloseFont(gFont); 

	//Destroy window	
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

bool checkCollision( Circle& a, SDL_Rect& b)
{
    //Closest point on collision box
    int cX, cY = 0;

    //Find closest x offset
    if( a.x < b.x )
    {
        cX = b.x;
    }
    else if( a.x > b.x + b.w )
    {
        cX = b.x + b.w;
    }
    else
    {
        cX = a.x;
    }

    //Find closest y offset
    if( a.y < b.y )
    {
        cY = b.y;
    }
    else if( a.y > b.y + b.h )
    {
        cY = b.y + b.h;
    }
    else
    {
        cY = a.y;
    }

    //If the closest point is inside the circle
    if( distanceSquared( a.x, a.y, cX, cY ) < a.r * a.r )
    {
        //This box and the circle have collided
        return true;
    }

    //If the shapes have not collided
    return false;
}

// Updates the brick.sidehit with the side that was hit. Return of true is success, return of false is that no side was hit
bool updateCollisionSide(Circle& a, brick& b)
{
	if(a.x < b.brickRect.x) // ball is to the left of the brick
	{	
		if(a.y < b.brickRect.y - b.brickRect.h/2) // ball is above the brick
		{
			//This is a top hit
			b.sidehit = TOP; return true; 
		}
		else if (a.y > b.brickRect.y + b.brickRect.h/2) // ball is below the brick
		{
			//This is a bottom hit
			b.sidehit = BOTTOM; return true; 
		}

		else
		{
			//Ball is neither below or above the brick. This is a left hit
			b.sidehit = LEFT; return true; 
		}
	}

	if(a.x > b.brickRect.x) // ball is to the right of the brick
	{
		if(a.y < b.brickRect.y - b.brickRect.h/2) // ball is above the brick
		{
			//This is a top hit
			b.sidehit = TOP; return true; 
		}
		else if (a.y > b.brickRect.y + b.brickRect.h/2) // ball is below the brick
		{
			//This is a bottom hit
			b.sidehit = BOTTOM; return true; 
		}

		else
		{
			//Ball is neither below or above the brick. This is a left hit
			b.sidehit = RIGHT; return true; 
		}

		return false; 
	}
}

double distanceSquared( int x1, int y1, int x2, int y2 )
{
	int deltaX = x2 - x1;
	int deltaY = y2 - y1;
	return deltaX*deltaX + deltaY*deltaY;
}

int main( int argc, char* args[] )
{
	//Start up SDL and create window
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
		//Load media
		if( !loadMedia() )
		{
			printf( "Failed to load media!\n" );
		}
		else
		{	
			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;

			// frames per second timer
			LTimer fpsTimer; 

			// in memory text stream
			std::stringstream fpsTimeText; 
			std::stringstream scoreText; 
			
			int countedFrames = 0; 
			fpsTimer.start(); 

			double avgFPS = 0; 

			

			// Maintain list of bricks in the game
			const int brickcount = 6; 

			// instantiate game objects
			paddle mainPaddle;
			
			//Create the playing field with numGameBricks, arrange them and make them random types. 
			std::vector<brick> gameBricks(numGameBricks);

			SDL_Rect mainGameViewport; 
			mainGameViewport.x = 0; 
			mainGameViewport.y = 0; 
			mainGameViewport.w = SCREEN_WIDTH; 
			mainGameViewport.h = SCREEN_HEIGHT - SCOREBOARD_HEIGHT; 
			// mainGameViewport.h = SCREEN_HEIGHT - SCOREBOARD_HEIGHT; 

			SDL_Rect ScoreBoardViewport; 
			ScoreBoardViewport.x = 0; 
			ScoreBoardViewport.y = SCREEN_HEIGHT - SCOREBOARD_HEIGHT; 
			ScoreBoardViewport.w = SCREEN_WIDTH; 
			ScoreBoardViewport.h = SCOREBOARD_HEIGHT; 

			for(int i = 0; i < gameBricks.size(); i++)
			{
				gameBricks[i].bricktype = rand() % 6; 

				if(i < 9)
				{
					gameBricks[i].arrange(80*i+40, 20);
				}
				else if (i >= 9 && i < 18)
				{
					gameBricks[i].arrange(80*(i-9)+40, 40);
				}
				else if (i >= 18 && i < 27)
				{
					gameBricks[i].arrange(80*(i-18)+40, 60); 
				}
				else
				{
					gameBricks[i].arrange(80*(i-27)+40, 80); 				
				}
			}

			ball mainBall;
			scoreboard mainScoreboard; 
			mainScoreboard.gamescore = 0; 

			mainBall.mPosX = SCREEN_WIDTH/2 - mainBall.ball_WIDTH/2; 
			mainBall.mPosY = SCREEN_HEIGHT - SCOREBOARD_HEIGHT - mainPaddle.paddle_height - mainBall.ball_HEIGHT/2; 

			// Game has started?
			bool gameOn = false; 

			//While application is running
			while( !quit )
			{
				//Handle events on queue
				while( SDL_PollEvent( &e ) != 0 )
				{
					//User requests quit
					if( e.type == SDL_QUIT )
					{
						quit = true;
					}

					//Handle input for the paddle
					mainPaddle.handleEvent( e );

					//Handle input for the paddle
					mainBall.handleEvent( e, gameOn);

					// Once the user presses space to start the game, set gameOn to prevent any further presses of "space" to change velocity.
					if(e.type = SDL_KEYDOWN)
					{
						switch( e.key.keysym.sym )
						{
							case SDLK_SPACE: 
								{
									gameOn = true;
								}
						}
					}
				}

				//Clear screen
				SDL_SetRenderDrawColor( gRenderer, 195, 195, 195, 0xFF );
				SDL_RenderClear( gRenderer );

				//Move the paddle
				mainPaddle.move();
				mainBall.move(gameBricks, mainPaddle);

				//Remove destroyed blocks if they exist
				for(int i = 0; i < gameBricks.size(); i++)
				{
					if(gameBricks[i].hitbyball == true)
					{
						gameBricks.erase(gameBricks.begin() + i);
						mainScoreboard.gamescore++; 
					}
				}

				// Switch to the main game viewport and render all objects
				SDL_RenderSetViewport(gRenderer, &mainGameViewport); 

				//Arrange and Render bricks
				for(brick b: gameBricks)
				{
					b.render(); 
				}

				mainPaddle.render(); 
				mainBall.render();
				
				// Switch to the scoreboard viewport and update the scoreboard
				SDL_RenderSetViewport(gRenderer, &ScoreBoardViewport);

				mainScoreboard.render(); 

				avgFPS = countedFrames / (fpsTimer.getTicks() / 1000.f);

				if (avgFPS > 2000000)
				{
					avgFPS = 0; 
				}

				fpsTimeText.str("");
				fpsTimeText << avgFPS; 

				gFPSTextTexture.loadFromRenderedText(fpsTimeText.str().c_str(), textColor);
				gFPSTextTexture.render(64 + 36, 128); 

				scoreText.str("");
				scoreText << mainScoreboard.gamescore; 
				gScoreTextTexture.loadFromRenderedText(scoreText.str().c_str(), textColor);
				gScoreTextTexture.render(64 + 52, 64);

				gFPSTextHeaderTexture.render(64, 128);
				gScoreTextHeaderTexture.render(64, 64);
				
				//Update screen
				SDL_RenderPresent( gRenderer );

				++countedFrames;
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}
/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>

//Screen dimension constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int numBricks = 6; 
const int numPaddles = 6; 
const int numBalls = 2; 

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
		static const int paddle_vel = 10;

		//Initializes the variables
		paddle();

		//Takes key presses and adjusts the paddles velocity
		void handleEvent( SDL_Event& e );

		//Moves the paddle
		void move();

		//Shows the brick on the screen
		void render();

    private:
		//The X and Y offsets
		int mPosX, mPosY;

		//The velocity of the paddle
		int mVelX, mVelY;
};

class brick
{
    public:
		//Brick dimensions
		static const int brick_width = 80;
		static const int brick_height = 20;

		SDL_Rect brickRect;

		//Initializes the variables
		brick();

		//Shows the brick on the screen
		int render(int idBrick);

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
		static const int ball_VEL = 4;

		//Initializes the variables
		ball();

		//Takes key presses and adjusts the ball
		void handleEvent( SDL_Event& e, bool gameOn);

		//Moves the ball
		void move(brick gameBricks[]);

		//Shows the ball on the screen
		void render();

    private:
		//The X and Y offsets of the ball
		int mPosX, mPosY;

		//The velocity of the ball
		int mVelX, mVelY;

		// ball collision circle
		Circle mBallCollider;

		////Moves the collision circle relative to the balls offset
		//void shiftColliders();
};


//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//Circle/Box collision detector
bool checkCollision( Circle& a, SDL_Rect& b );

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

//Clips

SDL_Rect gPaddleClips[numPaddles]; 
SDL_Rect gBrickClips[numBricks];
SDL_Rect gBallClips[numBalls];


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


paddle::paddle()
{
    //Initialize the paddle at the bottom middle
	mPosX = SCREEN_WIDTH/2 - (paddle_width/2); // in the middle 
    mPosY = SCREEN_HEIGHT- paddle_height; // bottom of the screen minus the height of the paddle

    //Initialize the velocity
    mVelX = 0;
    mVelY = 0;
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

    //If the paddle went too far to the left or right
    if( ( mPosX < 0 ) || ( mPosX + paddle_width> SCREEN_WIDTH ) )
    {
        //Move back
        mPosX -= mVelX;
    }

    ////Move the paddle up or down
    //mPosY += mVelY;

    ////If the paddle went too far up or down
    //if( ( mPosY < 0 ) || ( mPosY + paddle_height > SCREEN_HEIGHT ) )
    //{
    //    //Move back
    //    mPosY -= mVelY;
    //}
}

void paddle::render()
{
    //Show the paddle
		gPaddleTexture.render(mPosX, mPosY, &gPaddleClips[0]);
}


brick::brick()
{
	mPosX = 0; 
	mPosY = 0; 
}

int brick::render(int idBrick)
{
	if(idBrick <= numBricks-1)
	{
		gBrickTexture.render(mPosX, mPosY, &gBrickClips[idBrick]);
		return 0; 
	}
	else
	{
		printf("Selected invalid brick. There is not a brick with the ID %d\n", idBrick);
		return 1; 
	}
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
	mPosX = SCREEN_WIDTH/2-10;
	mPosY = SCREEN_HEIGHT-24-20;

	// Set collision circle size
	mBallCollider.r = ball_WIDTH/2; 

    //Initialize the velocity
    mVelX = 0;
    mVelY = 0;

	//// move collider relative to the circle
	//shiftColliders();
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

void ball::move(brick gameBricks[])
{
    //Move the ball left or right
    mPosX += mVelX;

    //If the ball went too far to the left or right or collided
	if( (mPosX - mBallCollider.r < 0) || (mPosX + mBallCollider.r > SCREEN_WIDTH) ||
		checkCollision(mBallCollider, gameBricks[0].brickRect) || checkCollision(mBallCollider, gameBricks[1].brickRect))
    {
        //Invert x velocity to make it bounce
        mPosX -= mVelX;
		mVelX = mVelX*-1;
    }

    //Move the ball up or down
    mPosY += mVelY;

    //If the ball went too far up or down
	if( ( mPosY - mBallCollider.r < 0 ) || ( mPosY + mBallCollider.r > SCREEN_HEIGHT) ||
		checkCollision(mBallCollider, gameBricks[0].brickRect) || checkCollision(mBallCollider, gameBricks[1].brickRect))
    {
        //Invert Y velocity to make it bounce
        mPosY -= mVelY;
		mVelY = -1*mVelY;
    }
}

void ball::render()
{
    //Show the ball
	gBallTexture.render( mPosX - mBallCollider.r, mPosY - mBallCollider.r, &gBallClips[0] );
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
		gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
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
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
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

	//Load paddle texture
	if( !gPaddleTexture.loadFromFile( "media/paddlesspritesheet.png" ) | !gBrickTexture.loadFromFile("media/bricksspritesheet.png")
		| !gBallTexture.loadFromFile("media/ballsspritesheet.png"))
	{
		printf( "Failed to load paddle sprite texture!\n" );
		success = false;
	}
	else
	{
		gPaddleClips[0].x = 0; 
		gPaddleClips[0].y = 0; 
		gPaddleClips[0].w = 200; 
		gPaddleClips[0].h = 24; 	


		for(int r = 0; r < numBricks; r++)
		{
			gBrickClips[r].x = 0; 
			gBrickClips[r].y = r*20; 
			gBrickClips[r].w = 80;
			gBrickClips[r].h = 20; 

		}

		gBallClips[0].x = 0; 
		gBallClips[0].y = 0; 
		gBallClips[0].w = 20;
		gBallClips[0].h = 20; 
		
	}


	return success;
}

void close()
{
	//Free loaded images
	gDotTexture.free();
	gBallTexture.free();
	gPaddleTexture.free();

	//Destroy window	
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

bool checkCollision( Circle& a, SDL_Rect& b )
{
    //Closest point on collision box
    int cX, cY;

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

			// Maintain list of bricks in the game
			const int brickcount = 6; 

			// instantiate game objects
			paddle mainPaddle;
			brick gameBricks[brickcount]; 
			ball mainBall;

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

				//Move the paddle
				mainPaddle.move();
				mainBall.move(gameBricks);

				//Clear screen
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
				SDL_RenderClear( gRenderer );

				//Arrange and Render bricks
				for(int i = 0; i < 6; i++)
				{
					gameBricks[i].arrange(i*80,0);
					gameBricks[i].render(i);
				}

				mainPaddle.render(); 
				mainBall.render();
				

				//Update screen
				SDL_RenderPresent( gRenderer );
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}
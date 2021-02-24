/*
 * Cannon with Crazy Bullets
 * This project was created by tagucha.
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <X11/Xutil.h>

#define BORDER 2
#define WIDTH 800
#define HEIGHT 520
#define BASE_HEIGHT 480
#define GRAVITY 0.1
#define GROUND 400
#define CANNON_X 120
#define SPEED 1.1
#define AIR_RESISTANCE 0.991
#define PI 3.14159265358979

//Simple functions
#define loop(i,n) for(i = 0;i < n;i++) //Simple loop
#define between(min,n,max) (min <= n && n <= max) //To compare function among three variables
#define min(a,b) (a<b?a:b) //To get the smaller variable than another
#define max(a,b) (a>b?a:b) //To get the larger variable than another
#define FillEclipse(x,y,r) XFillArc(display,window,gc,x-r,y-r,2*r,2*r,0,360*64) //To fill an eclipse simply
#define DrawRectangle(x,y,w,h) XDrawRectangle(display,window,gc,x,y,w,h) //To draw a rectangle simply
#define FillRectangle(x,y,w,h) XFillRectangle(display,window,gc,x,y,w,h) //To fill a rectangle simply
#define FillArea(minX,minY,maxX,maxY) FillRectangle(minX,minY,maxX-minX,maxY-minY) //To fill an area as a rectangle simply
#define DrawLine(x1,y1,x2,y2) XDrawLine(display,window,gc,x1,y1,x2,y2) //To draw line simply

const int tick_per_sec = 10000; //How many microseconds to update once
unsigned long ticks = 0; //Time since the game started

//To save used colors
typedef struct {
    unsigned short red,green,blue;
    unsigned long color;
} ColorMatch;

//Button with another window
typedef struct {
    int x,y;
    char name[8];
    Window layer;
} Button;

//Button without other windows
typedef struct {
    int x,y;
    int width,height;
    char name[16];
} SimpleButton;

//Block to object bullets
typedef struct {
    int minX,minY,maxX,maxY;
    int g_minX,g_maxX,g_maxY,g_minY;
    double dx,dy;
    int health, max_health;
} Block;

//Ball as bullets
typedef struct {
    double x,y,r,dx,dy;
    unsigned short red,green,blue;
    int state;
} Ball;

//Cannon to launch bullets
typedef struct {
    double x,y,dx,dy;
} Cannon;

//Explosion effect when bullets collide a block or an enemy
typedef struct {
    double x,y,r;
} Explosion;

//Enemy
typedef struct {
    double x,y,r;
    int max_health,health;
} Enemy;

Display *display;
Window window,root;
int screen;
unsigned long black, white;
GC gc;
XEvent event;
Button button_quit,button_menu; //Exit button and Menu button
SimpleButton stage_buttons[16]; //To select stage
ColorMatch color_match[128]; //To save used colors
int color_matched = 0; //How many colors were used

XFontSet FONT_NORMAL,FONT_TITLE,FONT_RESULT; //Fonts

Cannon cannon = {CANNON_X, GROUND, 0, 0}; //The cannon

int ball_num = 0; //How many bullets are flying
const int ball_count = 128; //How many bullets can exist once
Ball ball[ball_count]; //Bullets

int block_count = 0; //How many blocks exist
const int MAX_BLOCK_COUNT = 128; //How many blocks can exist once
Block blocks[MAX_BLOCK_COUNT]; //Blocks

int explosion_num = 0; //How many explosion effects exist
const int MAX_EXPLOSION_NUM = 256; //How many explosion effects can exist once
Explosion explosions[MAX_EXPLOSION_NUM]; //Explosion effects

int enemy_num = 0; //How many enemies exist
const int MAX_ENEMY_NUM = 16; //How many enemies can exist once
Enemy enemies[MAX_ENEMY_NUM]; //Enemies

int stage = 0; //Stage number
int launch_count = 0; //The number of launched balls.
int score = 0;
int scores[16] = {0};

void preInit(); //Calling first
void init(); //Calling secondary
void postInit(); //Calling thirdly
void createButton(Button *button, int x, int y, char *name); //To create a button (ex. Exit, Menu).
void createSimpleButton(SimpleButton *button, int x, int y, int width, int height, char *name); //To create a simple button (ex. stage select).

int draw(); //tick event (Continue:-1, SuccessfulExit:0)
void drawBackGround(); //Drawing background
void drawButton(Button *button); //Write the name of a button.
void drawStage(); //Draw the stage of the variable named stage.
void onChangeStage(int k); //Perform on changing to a stage whose id is k.
void changeStage(int k); //To change to a stage whose id is k.
void update(); //After drawing stage
void updateBall(Ball *ball); //To update the ball's location.
void updateExplosion(); //To update explosions' size.
void updateDeadEnemies(); //For remove dead enemies.
void checkCollision(Ball *bal, Block *block); //For checking collision.

void setColor(unsigned short r, unsigned short g, unsigned short b); //Simply setting the drawing color.
unsigned long getColor(unsigned short r, unsigned short g, unsigned short b); //RGB to unsigned long
void launch(); //To launch a bullet.
void collideBall(Ball *bal); //To initialize a ball;
void damageBlock(Block *block); //To damage which can be damaged.
void removeBlock(Block *block); //To remove which can be damaged.
void addBlock(int x, int y, int w, int h, int health); //To register a block on the stage.
void addMovingBlock(int x, int y, int w, int h, int health, double dx, double dy); //To register a moving block on the stage.
void addEnemy(int x, int y, int r, int health); //To register a enemy.
void drawEnemy(Enemy *enemy); //To draw the enemy.

//util
double distance(double x1, double y1, double x2, double y2); //To get the distance between two points.
void writeText(char *arg, int x, int y); //Simply drawing text.
void writeTextWithFont(char *arg, int x, int y, XFontSet font); //Simple drawing text with another font.
void writeTextWithFontOnStage(char *arg, int x, int y, XFontSet font); //Simple drawing text with another font on stage.
void FillLeanRectangle(double x, double y, double width, double height, double rad); //To draw a lean rectangle for the cannon.

int main() {
    preInit();
    init();
    postInit();

    printf("Loaded.\n");
    while(1) {
        if (XEventsQueued(display, QueuedAfterReading)) {
            XNextEvent(display, &event);
            int result = -1;
            if (event.type == ButtonPress) {
                if (event.xany.window == button_quit.layer) {
                    result = 0;
                }
            }
            if (result == -1) result = draw();
            if (result == -1) continue;
            printf("Exit (code:%d)\n",result);
            return 0;
        } else {
            drawStage();
            update();
        }
    }
}

void preInit() {
    printf("Loading...\n");

    display = XOpenDisplay("");
    root = DefaultRootWindow(display);
    screen = DefaultScreen(display);
    white = WhitePixel(display, screen);
    black = BlackPixel(display, screen);
    window = XCreateSimpleWindow(display, root, 100, 100, WIDTH, HEIGHT, BORDER, black, white);
    gc = XCreateGC(display, window, 0, NULL);

    XSelectInput(display, window, ButtonPressMask | PointerMotionMask);
}

void init() {
    printf("Initializing...\n");

    //create button
    createButton(&button_quit, 5, 2, "Exit");
    createButton(&button_menu, 5, 20, "Menu");

    const int borderX = 20, borderY = 30;
    int x,y,i;
    loop(x, 4) loop(y, 4) {
        char name[10];
        sprintf(name, "STAGE %d", y * 4 + x + 1);
        createSimpleButton(&stage_buttons[y * 4 + x], x * WIDTH / 4 + borderX, y * BASE_HEIGHT / 4 + borderY, WIDTH / 4 - borderX * 2, BASE_HEIGHT / 4 - borderY * 2,name);
    }
    loop(i,ball_count) {
        ball[i].x = CANNON_X;
        ball[i].y = GROUND;
        ball[i].r = 7;
        ball[i].dx = 0;
        ball[i].dy = 0;
        ball[i].red = 100;
        ball[i].blue = 60;
        ball[i].green = 250;
        ball[i].state = 0;
    }

    //register font
    char **miss,*def;
    int n_miss;
    FONT_NORMAL = XCreateFontSet(display,"-*-*-*-*-*-*-*-*-*-*-*-*-*-*",&miss,&n_miss,&def);
    FONT_TITLE = XCreateFontSet(display,"-*-*-bold-r-*-*-20-*-*-*-*-100-iso8859-*",&miss,&n_miss,&def);
    FONT_RESULT = XCreateFontSet(display,"-*-*-demibold-*-normal-*-34-*-*-*-*-*-*-*",&miss,&n_miss,&def);
}

void postInit() {
    printf("Waiting...\n");

    XMapWindow(display, window);
    XMapSubwindows(display, window);
}

void createButton(Button *button, int x, int y, char *name) {
    Window layer = XCreateSimpleWindow(display, window, x, y, 30, 12, BORDER, black, white);
    button->x = x;
    button->y = y;
    sprintf(button->name,"%s",name);
    button->layer = layer;
    XSelectInput(display,layer,ButtonPressMask);
    printf("The button named %s (id:%ld) was created.\n",name,layer);
}

void createSimpleButton(SimpleButton *button, int x, int y, int width, int height, char *name) {
    button->x = x;
    button->y = y;
    button->width = width;
    button->height = height;
    sprintf(button->name,"%s",name);
    printf("The simple button named %s was created.\n",button->name);
}

void drawButton(Button *button) {
    XmbDrawString(display, button->layer, FONT_NORMAL, gc, 2, 10, button->name, (int)strlen(button->name));
}

void drawStage() {
    XClearWindow(display, window);
    drawBackGround();
    if (stage == 0) {
        launch_count = 0;
        score = 0;
        setColor(255,255,255);
        writeTextWithFont("CANNON WITH CRAZY BULLETS",WIDTH / 2 - 135,30,FONT_TITLE);
        char arg[30];
        int sum = 0;
        int i;loop(i,16) sum += scores[i];
        sprintf(arg, "total: %d", sum);
        writeText(arg,WIDTH - 120,23);
        setColor(0,0,0);
        loop(i, 16) {
            int x = stage_buttons[i].x, y = stage_buttons[i].y;
            int h = stage_buttons[i].height / 2;
            writeTextWithFontOnStage(stage_buttons[i].name, stage_buttons[i].width / 2 - 36 + x,y + h, FONT_TITLE);
            char string[30];
            sprintf(string, "score: %d", scores[i]);
            writeTextWithFontOnStage(string, stage_buttons[i].width / 2 - 24 - strlen(string) + x,y + h + 16, FONT_NORMAL);
            DrawRectangle(x, y + HEIGHT - BASE_HEIGHT, stage_buttons[i].width, stage_buttons[i].height);
        }
    } else if (between(1,stage,16)) {
        int i;loop(i,ball_count) {
            updateBall(&ball[i]);
            setColor(ball[i].red,ball[i].green,ball[i].blue);
            FillEclipse(ball[i].x,ball[i].y,ball[i].r);
        }

        //ground
        setColor(0,0,0);
        FillArea(0,0,WIDTH,HEIGHT - BASE_HEIGHT);
        writeTextWithFont(stage_buttons[stage - 1].name,WIDTH / 2 - 40,30,FONT_TITLE);
        FillEclipse(cannon.x,cannon.y,30);
        double bias = 30 / sqrt(cannon.dx * cannon.dx + cannon.dy * cannon.dy);
        FillLeanRectangle(cannon.x + cannon.dx * bias,cannon.y + cannon.dy * bias,70,20,-atan2(cannon.dx,cannon.dy));
        FillArea(0,GROUND,WIDTH + 110,HEIGHT + 1000);
        FillArea(WIDTH + 110,0,WIDTH + 2000,HEIGHT + 1000);

        setColor(0,0,0);
        double time = ticks % 314159265;
        time /= 100;

        loop(i,block_count) {
            blocks[i].minX = blocks[i].g_minX + blocks[i].dx * cos(time);
            blocks[i].maxX = blocks[i].g_maxX + blocks[i].dx * cos(time);
            blocks[i].minY = blocks[i].g_minY + blocks[i].dy * sin(time);
            blocks[i].maxY = blocks[i].g_maxY + blocks[i].dy * sin(time);
            if (blocks[i].max_health > 0) {
                int red = 130 * (blocks[i].max_health - blocks[i].health + 1) / blocks[i].max_health;
                int green = 70 * (blocks[i].max_health - blocks[i].health + 1) / blocks[i].max_health;
                setColor(red, green, 0);
            } else{
                setColor(0,0,0);
            }
            FillArea(blocks[i].minX,blocks[i].minY,blocks[i].maxX,blocks[i].maxY);
        }
        loop(i,enemy_num) drawEnemy(&enemies[i]);

        int x,y;loop(x,ball_num){
            loop(y,block_count) checkCollision(&ball[x],&blocks[y]);
            loop(y,enemy_num) {
                double r = enemies[y].r * enemies[y].health / enemies[y].max_health;
                double d = enemies[y].r - r;
                if (distance(enemies[y].x,enemies[y].y + d,ball[x].x,ball[x].y) <= r + ball[x].r) {
                    enemies[y].health--;
                    score += enemies[y].max_health - enemies[y].health;
                    collideBall(&ball[x]);
                }
            }
        }

        setColor(255,255,255);
        writeTextWithFont(stage_buttons[stage - 1].name,WIDTH / 2 - 40,30,FONT_TITLE);
        char string[2][30];
        sprintf(string[0], "count: %d", launch_count);
        writeText(string[0],WIDTH - 100,15);
        sprintf(string[1], "point: %d", score);
        writeText(string[1],WIDTH - 100,30);

        updateExplosion();
        setColor(255,0,0);
        loop(i,explosion_num) FillEclipse(explosions[i].x,explosions[i].y,explosions[i].r);

        updateDeadEnemies();
    } else if (stage == 17) {
        setColor(255,255,255);
        writeTextWithFont("CANNON WITH CRAZY BULLETS",WIDTH / 2 - 135,30,FONT_TITLE);

        setColor(0,0,0);
        char arg[30];
        int sum = 0;
        int i;loop(i,16) sum += scores[i];
        sprintf(arg, "total: %d", sum);
        writeTextWithFont(arg,WIDTH / 2 - 70, 300, FONT_TITLE);

        //リサジュー曲線描画
        double time = ticks % 314159265;
        time /= 600;
        double w = WIDTH, h = HEIGHT, bh = BASE_HEIGHT;
        double biasX = 0.48, biasY = 0.46;
        writeTextWithFont("C",w / 2 + sin((time) * 10) * w * biasX,bh / 2 + cos((time) * 11) * bh * biasY + h - bh,FONT_TITLE);
        writeTextWithFont("L",w / 2 + sin((time - 0.02) * 10) * w * biasX,bh / 2 + cos((time - 0.02) * 11) * bh * biasY + h - bh,FONT_TITLE);
        writeTextWithFont("E",w / 2 + sin((time - 0.04) * 10) * w * biasX,bh / 2 + cos((time - 0.04) * 11) * bh * biasY + h - bh,FONT_TITLE);
        writeTextWithFont("A",w / 2 + sin((time - 0.06) * 10) * w * biasX,bh / 2 + cos((time - 0.06) * 11) * bh * biasY + h - bh,FONT_TITLE);
        writeTextWithFont("R",w / 2 + sin((time - 0.08) * 10) * w * biasX,bh / 2 + cos((time - 0.08) * 11) * bh * biasY + h - bh,FONT_TITLE);
        writeTextWithFont("C",w / 2 - sin((time) * 10) * w * biasX,bh / 2 - cos((time) * 11) * bh * biasY + h - bh,FONT_TITLE);
        writeTextWithFont("L",w / 2 - sin((time - 0.02) * 10) * w * biasX,bh / 2 - cos((time - 0.02) * 11) * bh * biasY + h - bh,FONT_TITLE);
        writeTextWithFont("E",w / 2 - sin((time - 0.04) * 10) * w * biasX,bh / 2 - cos((time - 0.04) * 11) * bh * biasY + h - bh,FONT_TITLE);
        writeTextWithFont("A",w / 2 - sin((time - 0.06) * 10) * w * biasX,bh / 2 - cos((time - 0.06) * 11) * bh * biasY + h - bh,FONT_TITLE);
        writeTextWithFont("R",w / 2 - sin((time - 0.08) * 10) * w * biasX,bh / 2 - cos((time - 0.08) * 11) * bh * biasY + h - bh,FONT_TITLE);

        writeTextWithFont("C L E A R",WIDTH / 2 - 90,240,FONT_RESULT);
    }
}

int draw() {
    Window win = event.xany.window;
    switch (event.type) {
        case ButtonPress:
            if (win == button_menu.layer) {
                changeStage(0);
            } else if (win == window) {
                int pX = event.xbutton.x, pY = event.xbutton.y-HEIGHT+BASE_HEIGHT;
                if (stage == 0) {
                    int i;
                    loop(i, 16) {
                        int minX = stage_buttons[i].x, minY = stage_buttons[i].y;
                        int maxX = stage_buttons[i].width + minX, maxY = stage_buttons[i].height + minY;
                        if (between(minX, pX, maxX) && between(minY, pY, maxY)) {
                            changeStage(i + 1);
                            break;
                        }
                    }
                } else {
                    if (event.xbutton.button == 1) {
                        if (cannon.dx > 0 && cannon.dy < 0) {
                            launch();
                        } else {
                            printf("The angle has an problem.\n");
                        }
                    }
                }
            }
            break;
        case MotionNotify:
            if (between(1,stage,16)) {
                double x = cannon.x - event.xmotion.x;
                double y = cannon.y - event.xmotion.y;
                double d = sqrt(x * x + y * y);
                double r = min(120,d);
                cannon.dx = x * r / d / 10;
                cannon.dy = y * r / d / 10;
            }
            break;
    }
    return -1;
}

void changeStage(int k) {
    printf("The stage was changed from %d to %d\n", stage, k);
    stage = k;
    onChangeStage(stage);
}

void onChangeStage(int k) {
    if (between(1,k,16)) {
        block_count = 0;
        enemy_num = 0;
        launch_count = 0;
        score = 0;
        ball_num = 0;
        addBlock(WIDTH + 100,0,10,HEIGHT,-1);
    }
    double x,y;
    switch (k) {
        case 1:
            //Clear check AC
            addEnemy(550,0,20,3);
            break;
        case 2:
            //Clear check AC
            addBlock(500,50,100,100,-1);
            addEnemy(700,0,30,5);
            addEnemy(550,0,20,3);
            break;
        case 3:
            //Clear check AC
            addBlock(500,0,100,50,-1);
            addBlock(600,0,100,100,-1);
            addBlock(700,0,100,150,-1);
            addEnemy(550,50,30,3);
            addEnemy(650,100,30,3);
            addEnemy(750,150,30,3);
            break;
        case 4:
            //Clear check AC
            addBlock(500,0,100,100,-1);
            addBlock(700,0,100,100,-1);
            addEnemy(550,100,30,3);
            addEnemy(650,0,50,10);
            addEnemy(750,100,30,3);
            break;
        case 5:
            //Clear check AC
            addBlock(400,0,100,50,-1);
            addBlock(600,0,100,150,-1);
            addEnemy(450,50,30,3);
            addEnemy(550,0,40,5);
            addEnemy(650,150,30,3);
            addEnemy(750,0,50,10);
            break;
        case 6:
            //Clear check AC
            addBlock(400,200,100,50,-1);
            addBlock(400,0,100,120,-1);
            addEnemy(450,250,30,4);
            addEnemy(450,120,30,4);
            addEnemy(550,0,50,15);
            addEnemy(650,0,50,10);
            addEnemy(750,0,50,5);
            break;
        case 7:
            //Clear check AC
            loop(x,3)loop(y,10) addBlock(300 + x * 35,y * 35,2,2,-1);
            addEnemy(700,0,100,20);
            break;
        case 8:
            //Clear check AC
            loop(x, 10) loop(y,3) addBlock(800 - cos(x / 20 * PI) * (200 + y * 50),sin(x / 20 * PI) * (200 + y * 50),3,3,-1);
            addEnemy(700,0,100,20);
            break;
        case 9:
            //Clear check AC
            addBlock(500,0,100,350,3);
            addEnemy(700,0,30,5);
            break;
        case 10:
            //Clear check AC
            loop(x,5) loop(y,6) addBlock(x * 50 + 400, y * 50 + 50,50,50,3);
            addBlock(400,0,50,50,5);
            addBlock(500,0,50,50,5);
            addBlock(600,0,50,50,5);
            addEnemy(475,0,25,5);
            addEnemy(575,0,25,5);
            addEnemy(720,0,50,10);
            break;
        case 11:
            //Clear check AC
            addBlock(400,0,50,80,-1);
            addBlock(400,110,50,90,-1);
            addBlock(600,0,50,200,-1);
            addBlock(450,150,150,50,10);
            addEnemy(525,0,50,20);
            addEnemy(525,200,30,5);
            break;
        case 12:
            //Clear check AC
            addBlock(600,0,70,150,10);
            addMovingBlock(500,-75,100,200,-1,0,75);
            addMovingBlock(WIDTH-50,150,300,50,-1,150,0);
            addEnemy(700,0,30,15);
            break;
        case 13:
            //Clear check AC
            addMovingBlock(500,-75,100,200,-1,0,75);
            addMovingBlock(500,275,100,200,-1,0,-75);
            addMovingBlock(400,-75,100,200,-1,0,-75);
            addMovingBlock(400,275,100,200,-1,0,75);
            addBlock(725,0,75,200,-1);
            addEnemy(700,0,25,10);
            addEnemy(762,200,30,10);
            break;
        case 14:
            //Clear check AC
            loop(x,4) loop(y,3) addMovingBlock(300 + 100 * x,25 + y * 100,50,50,1,0,25);
            loop(x,4) loop(y,3) addMovingBlock(300 + 100 * x,25 + y * 100,50,50,2,25,0);
            loop(x,4) loop(y,3) addMovingBlock(250 + 100 * x,25 + y * 100,50,50,1,0,-25);
            loop(x,4) loop(y,3) addMovingBlock(250 + 100 * x,25 + y * 100,50,50,2,-25,0);
            addEnemy(750,0,50,15);
            addEnemy(750,120,50,15);
            addEnemy(750,240,50,15);
            addBlock(700,100,100,20,-1);
            addBlock(700,220,100,20,-1);
            break;
        case 15:
            //Clear check AC
            loop(x,4) loop(y,8) addMovingBlock(300 + 50 * x,50 * y,50,50,-1,((int)x % 2 == 0 ? 1 : -1) * 50,((int)y % 2 == 1 ? 1 : -1) * 50);
            addEnemy(750,0,50,15);
            break;
        case 16:
            //Clear check
            addEnemy(550,0,200,100);
            break;
    }
}

void drawBackGround() {
    setColor(0,0,0);
    drawButton(&button_quit);
    drawButton(&button_menu);
    DrawLine( 0, HEIGHT - BASE_HEIGHT, WIDTH, HEIGHT - BASE_HEIGHT);
    DrawLine(0, 0,0, HEIGHT);
    DrawLine(WIDTH, 0, WIDTH, HEIGHT);
    DrawLine(0, HEIGHT, WIDTH, HEIGHT);
    FillArea(0,0,WIDTH,HEIGHT - BASE_HEIGHT);
}

void update() {
    usleep(tick_per_sec);
    ticks++;
    XFlush(display);
}

void updateBall(Ball *bal) {
    if (bal->y - bal->r <= GROUND - bal->r) {
        bal->x += bal->dx * SPEED;
        bal->y += (bal->dy += GRAVITY) * SPEED;
        //Air resistance
        bal->dx *= AIR_RESISTANCE;
        bal->dy *= AIR_RESISTANCE;
    } else if (bal->state == 1){
        collideBall(bal);
    }
}

void updateExplosion() {
    int i;
    loop(i,explosion_num) {
        explosions[i].r -= 10;
    }
    int c = 0;
    loop(i,explosion_num) {
        if (explosions[i].r > 0) {
            explosions[c] = explosions[i];
            c++;
        }
    }
    explosion_num = c;
}

void updateDeadEnemies() {
    int i,c=0;loop(i,enemy_num) {
        if (enemies[i].health > 0) {
            enemies[c] = enemies[i];
            c++;
        }
    }
    enemy_num = c;
    if (enemy_num == 0 && between(1,stage,16)) {
        scores[stage - 1] = score * 100 - launch_count * launch_count;
        changeStage(stage + 1);
    }
}

void checkCollision(Ball *bal, Block *block) {
    double bX = bal->x, bY = bal->y, bR = bal->r;
    int minX = block->minX, minY = block->minY, maxX = block->maxX, maxY = block->maxY;
    if (between(minX, bX, maxX)) {
        if (between(minY - bR, bY,maxY + bR)) {
            printf("collide Y %f %f %d %d %d %d %ld\n",bX,bY,minX,minY,maxX,maxY,block);
            collideBall(bal);
            damageBlock(block);
        }
    } else if (between(minY, bY, maxY)) {
        if (between(minX - bR, bX, maxX + bR)) {
            printf("collide X %f %f %d %d %d %d %ld\n",bX,bY,minX,minY,maxX,maxY,block);
            collideBall(bal);
            damageBlock(block);
        }
    }
    else if (distance(bX,bY,minX,minY) <= bR) {
        printf("collide ii %f %f %d %d\n",bX,bY,minX,minY);
        collideBall(bal);
        damageBlock(block);
    } else if (distance(bX,bY,minX,maxY) <= bR) {
        printf("collide ia %f %f %d %d\n",bX,bY,minX,maxY);
        collideBall(bal);
        damageBlock(block);
    } else if (distance(bX,bY,maxX,minY) <= bR) {
        printf("collide ai %f %f %d %d\n",bX,bY,maxX,minY);
        collideBall(bal);
        damageBlock(block);
    } else if (distance(bX,bY,maxX,maxY) <= bR) {
        printf("collide aa %f %f %d %d\n",bX,bY,maxX,maxY);
        collideBall(bal);
        damageBlock(block);
    }
}



void setColor(unsigned short r, unsigned short g, unsigned short b) {
    XSetForeground(display, gc, getColor(r,g,b));
}

unsigned long getColor(unsigned short r, unsigned short g, unsigned short b) {
    if (r == 0 && g == 0 && b == 0) return black;
    else if (r == 255 && g == 255 && b == 255) return white;
    else {
        int i;loop(i,color_matched) if (color_match[i].red == r && color_match[i].green == g && color_match[i].blue == b) return color_match[i].color;
        Colormap map = DefaultColormap(display, screen);
        XColor c0,c1;
        char color_string[16];
        sprintf(color_string, "rgb:%02X/%02X/%02X", r, g, b);
        XAllocNamedColor(display, map, color_string, &c1, &c0);
        color_match[color_matched].red = r;
        color_match[color_matched].green = g;
        color_match[color_matched].blue = b;
        color_match[color_matched].color = c1.pixel;
        return color_match[color_matched++].color;
    }
}

void launch() {
    if (between(1,stage,16)) {
        ball[ball_num].dx = cannon.dx * SPEED;
        ball[ball_num].dy = cannon.dy * SPEED;
        ball[ball_num].x = cannon.x;
        ball[ball_num].y = cannon.y;
        ball[ball_num].state = 1;
        ball_num = ball_num < ball_count - 1 ? ball_num + 1 : 0;
        launch_count++;
    }
}

void collideBall(Ball *bal) {
    if (explosion_num < MAX_EXPLOSION_NUM) {
        explosions[explosion_num].x = bal->x;
        explosions[explosion_num].y = bal->y;
        explosions[explosion_num].r = 100;
        explosion_num++;
    }
    bal->dx = 0;
    bal->dy = 0;
    bal->x = cannon.x;
    bal->y = cannon.y;
    bal->state = 0;
}

void damageBlock(Block *block) {
    if (block->max_health < 0) return;
    block->health--;
    if (block->health <= 0) {
        removeBlock(block);
        printf("Break the block whose id is %ld\n",block);
    } else {
        printf("Damage the block whose id is %ld (%d/%d)\n",block,block->health,block->max_health);
    }
}

void removeBlock(Block *block) {
    int i,c=0;loop(i,block_count) if (&blocks[i] != block) {
        blocks[c] = blocks[i];
        c++;
    }
    block_count = c;
}

void addBlock(int x, int y, int w, int h, int health) {
    if (block_count == MAX_BLOCK_COUNT) return;
    blocks[block_count].minX = x;
    blocks[block_count].g_minX = x;
    blocks[block_count].minY = GROUND - h - y;
    blocks[block_count].g_minY = GROUND - h - y;
    blocks[block_count].maxX = x + w;
    blocks[block_count].g_maxX = x + w;
    blocks[block_count].maxY = GROUND - y;
    blocks[block_count].g_maxY = GROUND - y;
    blocks[block_count].dx = 0;
    blocks[block_count].dy = 0;
    blocks[block_count].health = health;
    blocks[block_count].max_health = health;
    printf("Register a block %d %d %d %d %ld\n",blocks[block_count].minX,blocks[block_count].minY,blocks[block_count].maxX,blocks[block_count].maxY,&blocks[block_count]);
    block_count++;
}

void addMovingBlock(int x, int y, int w, int h, int health, double dx, double dy) {
    if (block_count == MAX_BLOCK_COUNT) return;
    blocks[block_count].minX = x;
    blocks[block_count].g_minX = x;
    blocks[block_count].minY = GROUND - h - y;
    blocks[block_count].g_minY = GROUND - h - y;
    blocks[block_count].maxX = x + w;
    blocks[block_count].g_maxX = x + w;
    blocks[block_count].maxY = GROUND - y;
    blocks[block_count].g_maxY = GROUND - y;
    blocks[block_count].dx = dx;
    blocks[block_count].dy = dy;
    blocks[block_count].health = health;
    blocks[block_count].max_health = health;
    printf("Register a moving block %d %d %d %d %ld dx=%f dy=%f\n",blocks[block_count].minX,blocks[block_count].minY,blocks[block_count].maxX,blocks[block_count].maxY,&blocks[block_count],dx,dy);
    block_count++;
}

void addEnemy(int x, int y, int r, int health) {
    if (enemy_num == MAX_ENEMY_NUM) return;
    enemies[enemy_num].x = x;
    enemies[enemy_num].y = GROUND - y - r;
    enemies[enemy_num].r = r;
    enemies[enemy_num].health = health;
    enemies[enemy_num].max_health = health;
    enemy_num++;
}

void drawEnemy(Enemy *enemy) {
    setColor(0,0,0);
    double r = enemy->r * enemy->health / enemy->max_health;
    double d = enemy->r - r;
    FillEclipse(enemy->x,enemy->y + d,r);
    setColor(255,(255 - max(0,min(255,15 * enemy->max_health))),(255 - max(0,min(255,26 * enemy->max_health))));
    double er = r / 3;
    FillEclipse(enemy->x - r / 2,enemy->y - r / 3 + d,er);
    FillEclipse(enemy->x + r / 2,enemy->y - r / 3 + d,er);
    DrawRectangle(enemy->x - er,enemy->y + r / 3 + d,er * 2,er * cos(((double) ticks) * 0.03) * cos(((double) ticks) * 0.03));
    setColor(0,0,0);
    FillEclipse(enemy->x - r / 2 + sin(((double) ticks) * 0.03) * er / 2,enemy->y - r / 3 + d,er / 2);
    FillEclipse(enemy->x + r / 2 + sin(((double) ticks) * 0.03) * er / 2,enemy->y - r / 3 + d,er / 2);
}

//以下util
double distance(double x1, double y1, double x2, double y2) {
    double x = x2 - x1, y = y2 - y1;
    return sqrt(x * x + y * y);
}

void writeText(char *arg, int x, int y) {
    writeTextWithFont(arg,x,y,FONT_NORMAL);
}

void writeTextWithFont(char *arg, int x, int y, XFontSet font) {
    XmbDrawString(display,window,font,gc,x,y,arg,(int)strlen(arg));
}

void writeTextWithFontOnStage(char *arg, int x, int y, XFontSet font) {
    XmbDrawString(display,window,font,gc,x,y+HEIGHT-BASE_HEIGHT,arg,(int)strlen(arg));
}

void FillLeanRectangle(double x, double y, double width, double height, double rad) {
    double minX = -width / 2, maxX = width / 2;
    double minY = -height / 2, maxY = height / 2;
    double r = sqrt(width * width + height * height) / 2;
    XPoint points[4];
    points[0].x = cos(atan2(minX,minY) + rad) * r + x;
    points[1].x = cos(atan2(minX,maxY) + rad) * r + x;
    points[2].x = cos(atan2(maxX,maxY) + rad) * r + x;
    points[3].x = cos(atan2(maxX,minY) + rad) * r + x;
    points[0].y = sin(atan2(minX,minY) + rad) * r + y;
    points[1].y = sin(atan2(minX,maxY) + rad) * r + y;
    points[2].y = sin(atan2(maxX,maxY) + rad) * r + y;
    points[3].y = sin(atan2(maxX,minY) + rad) * r + y;
    XFillPolygon(display,window,gc,points,4,Complex,CoordModeOrigin);
}
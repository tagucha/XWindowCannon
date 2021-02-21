/*
 * Breakout with Falling Balls
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

#define loop(i,n) for(i = 0;i < n;i++)
#define between(min,n,max) (min <= n && n <= max)
#define min(a,b) (a<b?a:b)
#define max(a,b) (a>b?a:b)
#define sandwich(min,n,max) (min>n?min:(max<n?max:n))
#define DrawEclipse(x,y,r) XDrawArc(display,window,gc,x-r,y-r,2*r,2*r,0,360*64)
#define FillEclipse(x,y,r) XFillArc(display,window,gc,x-r,y-r,2*r,2*r,0,360*64)
#define DrawRectangle(x,y,w,h) XDrawRectangle(display,window,gc,x,y,w,h)
#define FillRectangle(x,y,w,h) XFillRectangle(display,window,gc,x,y,w,h)
#define FillArea(minX,minY,maxX,maxY) FillRectangle(minX,minY,maxX-minX,maxY-minY)
#define DrawRectangleOnStage(x,y,w,h) DrawRectangle(x,y+HEIGHT-BASE_HEIGHT,w,h);
#define DrawPoint(x,y) XDrawPoint(display,window,gc,x,y)
#define DrawLine(x1,y1,x2,y2) XDrawLine(display,window,gc,x1,y1,x2,y2)

//const int tick_per_sec = 50000;
const int tick_per_sec = 10000;
//const int tick_per_sec = 500000;
unsigned long ticks = 0;

typedef struct {
    unsigned short red,green,blue;
    unsigned long color;
} ColorMatch;

typedef struct {
    int x,y;
    char *name;
    Window layer;
} Button;

typedef struct {
    int x,y;
    int width,height;
    char *name;
} SimpleButton;

typedef struct {
    int maxX,maxY,minX,minY;
} Block;

typedef struct {
    double x,y,r,dx,dy;
    Block *preBlock;
    unsigned short red,green,blue;
} Ball;

typedef struct {
    double x,y,dx,dy;
} Cannon;

Display *display;
Window window,root;
int screen;
unsigned long black, white;
GC gc;
XEvent event;
KeySym key;
Button button_quit,button_menu,button_red,button_blue,button_green;
SimpleButton stage_buttons[16];
int colorR = 0, colorG = 0, colorB = 0;
ColorMatch color_match[100];
int color_matched = 0;

XFontSet FONT_NORMAL;
XFontSet FONT_TITLE;

Block NULL_BLOCK = {-1,-1,-1,-1};
Cannon cannon = {CANNON_X, GROUND, 0, 0};

int ball_num = 0;
const int ball_count = 30;
Ball ball[ball_count];
//Bar bar = {WIDTH / 2,BASE_HEIGHT - 50,70,10};

char button_name[16][10];

int stage = 0; //Stage number

void preInit(); //Calling first
void init(); //Calling secondary
void postInit(); //Calling thirdly
void createButton(Button *button, int x, int y, char *name); //To create a button (ex. Exit, Menu).
void createSimpleButton(SimpleButton *button, int x, int y, int width, int height, char *name); //To create a simple button (ex. stage select).

int draw(); //tick event (Continue:-1, SuccessfulExit:0)
void drawBackGround(); //Drawing background
void drawButton(Button *button); //Write the name of a button.
void drawStage(); //Draw the stage of the variable named stage.
void update(); //After drawing stage
void updateBall(Ball *ball);

void writeText(char *arg, int x, int y); //Simply drawing text.
void writeTextWithFont(char *arg, int x, int y, XFontSet font); //Simple drawing text with another font.
void writeTextWithFontOnStage(char *arg, int x, int y, XFontSet font); //Simple drawing text with another font on stage.
void setColor(unsigned short r, unsigned short g, unsigned short b); //Simply setting the drawing color.
unsigned long getColor(unsigned short r, unsigned short g, unsigned short b); //RGB to unsigned long
double distance(double x1, double y1, double x2, double y2); //To get the distance between two points.
void FillLeanRectangle(double x, double y, double width, double height, double rad); //To draw a lean rectangle for the cannon.
void launch(); //To launch a bullet.

int main() {
    preInit();
    init();
    postInit();

//    return 0;

    printf("Loaded.\n");
    while(1) {
        if (XEventsQueued(display, QueuedAfterReading)) {
            XNextEvent(display, &event);
//            printf("type=%d window=%ld\n",event.type,event.xany.window);
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
        sprintf(button_name[y * 4 + x], "STAGE %d", y * 4 + x + 1);
        createSimpleButton(&stage_buttons[y * 4 + x], x * WIDTH / 4 + borderX, y * BASE_HEIGHT / 4 + borderY, WIDTH / 4 - borderX * 2, BASE_HEIGHT / 4 - borderY * 2,button_name[y * 4 + x]);
//        printf("%s",stage_buttons[y * 4 + x].name);
    }
    loop(i,ball_count) {
        ball[i].x = CANNON_X;
        ball[i].y = GROUND;
        ball[i].r = 7;
        ball[i].dx = 0;
        ball[i].dy = 0;
        ball[i].preBlock = &NULL_BLOCK;
        unsigned int c = (unsigned int)(((double)i) * 0xFFFFFF / ((double)ball_count));
        ball[i].red = (c >> 16) & 0xFF;
        ball[i].green = (c >> 8) & 0xFF;
        ball[i].blue = c & 0xFF;
    }

    //register font
    char **miss,*def;
    int n_miss;
    FONT_NORMAL = XCreateFontSet(display,"-*-*-*-*-*-*-*-*-*-*-*-*-*-*",&miss,&n_miss,&def);
    FONT_TITLE = XCreateFontSet(display,"-*-*-bold-r-*-*-20-*-*-*-*-100-iso8859-*",&miss,&n_miss,&def);
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
    button->name = name;
    button->layer = layer;
    XSelectInput(display,layer,ButtonPressMask);
    printf("The button named %s (id:%ld) was created.\n",name,layer);
}

void createSimpleButton(SimpleButton *button, int x, int y, int width, int height, char *name) {
    button->x = x;
    button->y = y;
    button->width = width;
    button->height = height;
    button->name = name;
    printf("The simple button named %s was created.\n",button->name);
}

void drawButton(Button *button) {
    XmbDrawString(display, button->layer, FONT_NORMAL, gc, 2, 10, button->name, (int)strlen(button->name));
//    printf("The button named %s was drawn.\n",button->name);
}

void drawStage() {
    XClearWindow(display, window);
    drawBackGround();
    if (stage == 0) {
        setColor(0,0,0);
        writeTextWithFont("CANNON WITH CRAZY BULLETS",WIDTH / 2 - 135,30,FONT_TITLE);
        int i;
        loop(i, 16) {
            int x = stage_buttons[i].x, y = stage_buttons[i].y;
            int h = stage_buttons[i].height / 2 + 5;
            writeTextWithFontOnStage(stage_buttons[i].name, stage_buttons[i].width / 2 - 36 + x,y + h, FONT_TITLE);
            DrawRectangleOnStage(x, y, stage_buttons[i].width, stage_buttons[i].height);
        }
    } else if (between(1,stage,16)) {
        setColor(0,0,0);
        writeTextWithFont(stage_buttons[stage - 1].name,WIDTH / 2 - 40,30,FONT_TITLE);

        int i;loop(i,ball_count) {
            updateBall(&ball[i]);
            setColor(ball[i].red,ball[i].green,ball[i].blue);
            FillEclipse(ball[i].x,ball[i].y,ball[i].r);
        }

        //ground
        setColor(0,0,0);
        writeTextWithFont(stage_buttons[stage - 1].name,WIDTH / 2 - 40,30,FONT_TITLE);
        FillEclipse(cannon.x,cannon.y,30);
        double bias = 30 / sqrt(cannon.dx * cannon.dx + cannon.dy * cannon.dy);
        FillLeanRectangle(cannon.x + cannon.dx * bias,cannon.y + cannon.dy * bias,70,20,-atan2(cannon.dx,cannon.dy));
        FillArea(0,GROUND,WIDTH,HEIGHT);
    }
}

int draw() {
    Window win = event.xany.window;
    switch (event.type) {
        case ButtonPress:
            if (win == button_menu.layer) {
                printf("The stage was changed from %d to %d\n",stage,0);
//                ball.dx = 0;
//                ball.dy = 0;
                stage = 0;
            } else if (win == window) {
                int pX = event.xbutton.x, pY = event.xbutton.y-HEIGHT+BASE_HEIGHT;
                printf("X=%d Y=%d\n",pX,pY);
                if (stage == 0) {
                    int i;
                    loop(i, 16) {
                        int minX = stage_buttons[i].x, minY = stage_buttons[i].y;
                        int maxX = stage_buttons[i].width + minX, maxY = stage_buttons[i].height + minY;
                        if (between(minX, pX, maxX) && between(minY, pY, maxY)) {
                            printf("The stage was changed from %d to %d\n", stage, i + 1);
                            stage = i + 1;
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
                double r = min(100,d);
                cannon.dx = x * r / d / 10;
                cannon.dy = y * r / d / 10;
            }
            break;
    }
    return -1;
}

void drawBackGround() {
    setColor(0,0,0);
    drawButton(&button_quit);
    drawButton(&button_menu);
    DrawLine( 0, HEIGHT - BASE_HEIGHT, WIDTH, HEIGHT - BASE_HEIGHT);
    DrawLine(0, 0,0, HEIGHT);
    DrawLine(WIDTH, 0, WIDTH, HEIGHT);
    DrawLine(0, HEIGHT, WIDTH, HEIGHT);
}

void update() {
    usleep(tick_per_sec);
    ticks++;
    XFlush(display);
}

void updateBall(Ball *bal) {
    if (bal->y - bal->r <= GROUND) {
        bal->x += bal->dx * SPEED;
        bal->y += (bal->dy += GRAVITY) * SPEED;
    }
}

void checkCollision(Ball *bal, Block *block) {
    double bX = bal->x, bY = bal->y, bR = bal->r, dX = bal->dx, dY = bal->dy;
    int minX = block->minY, minY = block->minY, maxX = block->maxX, maxY = block->maxY;
    double range = sqrt(dX * dX + dY * dY);
    double dist;
    if (between(minX, bX, maxX)) {
        if (bal->preBlock == block) return;
        if (bY - maxY <= bR || minY - bY <= bR) {
            bal->dy *= -1;
            bal->preBlock = block;
        }
    } else if (between(minY, bY, maxY)) {
        if (bal->preBlock == block) return;
        if (bX - maxX <= bR || minX - bX <= bR) {
            bal->dx *= -1;
            bal->preBlock = block;
        }
    }
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
//        printf("aaa %ld\n",c1.pixel);
        color_match[color_matched].red = r;
        color_match[color_matched].green = g;
        color_match[color_matched].blue = b;
        color_match[color_matched].color = c1.pixel;
        return color_match[color_matched++].color;
    }
}

double distance(double x1, double y1, double x2, double y2) {
    double x = x2 - x1, y = y2 - y1;
    return sqrt(x * x + y * y);
}

void launch() {
    ball[ball_num].dx = cannon.dx * SPEED;
    ball[ball_num].dy = cannon.dy * SPEED;
    ball[ball_num].x = cannon.x;
    ball[ball_num].y = cannon.y;
    ball_num = ball_num < ball_count - 1 ? ball_num + 1 : 0;
}

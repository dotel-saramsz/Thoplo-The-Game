#include<windows.h>
#include<winbase.h>
#include<commctrl.h>
#include<stdio.h>
#include<math.h>
#include<time.h>
#include<GL/gl.h>
#include<GL/glu.h>
#include "resource.h"

#define DEFAULT_BUTTON_WIDTH 124
#define DEFAULT_BUTTON_HEIGHT 40



HINSTANCE GlobalInstance;
HMENU Menu;
HMENU PopupMenu;
HWND Window;
HWND RenderingWindow;
HWND bPlayGame;
HWND bStoryMode;
HWND bHighScores;
HWND bAbout;
HWND bInstructions;
HWND bExit;
HWND bReplay;
HWND bMainMenu;
RECT rect;
HDC hDC;
HDC tempDC;
HGLRC glRC;
MSG msg;

int movement=0;
int enemydetection=0;  //The flag which is activated to 1 when enemy is detected or collided.
int coinelimination=0; //The flag to check if a coin is detected or not.
int display_certificate;    // The Window variable to show or hide the window.
int ThoploLives=3;
int game_over=0;
int game_over_count=0; // so as to enter the score into the file only once
int GuestNumber=0;
int menuchoice=0;
int presentLevel=1;
int menuentry=0;        // Flags or counters
float h=0.1;            // The thoplo's x-coordinate tracker
float k=1.85;            // The thoplo's y-coordinate tracker
int startx,starty,lengthx,lengthy; // Rendering Window co-ordinates

char *RunTimeAction="                             " ;
char PlayerName[50];
char CurrentUserScore[50]; // All Time High Score of Logged In User


struct scoreInfo
{
    int coinPoints;
    int enemyCollision;
    int GameOverScore;
    int totalPoints;
}score;

struct highscoreInfo
    {
        int score;
        char name[50];
    };

struct userInfo
    {
        char name[50];
        char password[50];
        int highscore;
    };

// Coin co-ordinates.
static float l1coinx[]={1.84,0.15,0.65,1.84,0.15,0.4},l1coiny[]={1.65,1.65,1.25,0.75,0.85,0.15},l2coinx[]={0.15,0.25,1.05,1.45,0.15,0.15},l2coiny[]={1.65,1.45,1.25,0.75,0.45,0.15};

FILE *User;
FILE *HighScores;




void EnableOpenGL(HWND hWnd, HDC* hDC,HGLRC* glRC)
{
    *hDC=GetDC(hWnd); // Getting Device Context

    PIXELFORMATDESCRIPTOR pfd;
    int Pixelformat;

    memset(&pfd,0,sizeof(pfd));
    pfd.nSize=sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion=1;
    pfd.dwFlags=PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
    pfd.cColorBits=24;
    pfd.cDepthBits=32;

    Pixelformat=ChoosePixelFormat(*hDC,&pfd);
    SetPixelFormat(*hDC,Pixelformat,&pfd);

    *glRC=wglCreateContext(*hDC);

    wglMakeCurrent(*hDC,*glRC);
}

void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC glRC)
{

    wglMakeCurrent(NULL,NULL);

    wglDeleteContext(glRC);

    ReleaseDC(hWnd,hDC);

}

void ResizeGLWindow(long width, long height)
{
    glViewport(0,0,(GLsizei)width,(GLsizei)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    GLfloat aspect= (GLfloat)width/(GLfloat)height ;

     if (width >= height)
        {
     // aspect >= 1, set the height from -1 to 1, with larger width
            gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
        }
     else
        {
      // aspect < 1, set the width to -1 to 1, with larger height
            gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
        }
                                           // Change and see later on
    glMatrixMode(GL_MODELVIEW);

}

void SetGLDefaults()
{
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
}

float wndCoord(float x) // To convert OpenGL co-ordinates into Windows co-ordinates for feeding glReadPixels
{
    float converted=28*x/0.1;
    return converted;
}

void Resetcoordinates(float *ph, float *pk)
{
    *ph=0.1;
    *pk=1.85;
}

int EliminateCoin(float h,float k,float coinx[],float coiny[]) // To check which coin is eaten so as to eliminate it from the screen
{
    if((h>=coinx[0]-0.05 && h<=coinx[0]+0.05) && (k>=coiny[0]-0.05 && k<=coiny[0]+0.05)) return 0;
    else if((h>=coinx[1]-0.05 && h<=coinx[1]+0.05) && (k>=coiny[1]-0.05 && k<=coiny[1]+0.05)) return 1;
    else if((h>=coinx[2]-0.05 && h<=coinx[2]+0.05) && (k>=coiny[2]-0.05 && k<=coiny[2]+0.05)) return 2;
    else if((h>=coinx[3]-0.05 && h<=coinx[3]+0.05) && (k>=coiny[3]-0.05 && k<=coiny[3]+0.05)) return 3;
    else if((h>=coinx[4]-0.05 && h<=coinx[4]+0.05) && (k>=coiny[4]-0.05 && k<=coiny[4]+0.05)) return 4;
    else if((h>=coinx[5]-0.05 && h<=coinx[5]+0.05) && (k>=coiny[5]-0.05 && k<=coiny[5]+0.05)) return 5;
    else if((h>=coinx[6]-0.05 && h<=coinx[6]+0.05) && (k>=coiny[6]-0.05 && k<=coiny[6]+0.05)) return 6;
    else return -1;

}

int MoveValidity(float h, float k, int direction)// To check if the thoplo can go to the specified location or not
{
    GLubyte pixelcolor[3];
    glReadBuffer(GL_BACK);

    switch(direction)
    {
    case 1:// UP
      {
        glReadPixels(wndCoord(h),wndCoord(k+0.01),1,1,GL_RGB,GL_UNSIGNED_BYTE,&pixelcolor);

        if(pixelcolor[0]==0 && pixelcolor[1]==255 && pixelcolor[2]==255)
        {
            return(0);
        }
        else if(pixelcolor[0]==255 && pixelcolor[1]==0 && pixelcolor[2]==255)
        {
            enemydetection=1;
        }
        else if(pixelcolor[0]==255 && pixelcolor[1]==255 && pixelcolor[2]==0)
        {
            coinelimination=1;
        }
        else
        {
            return(1);
        }
        break;
      }
    case 2:// DOWN
        {
            glReadPixels(wndCoord(h),wndCoord(k-0.01),1,1,GL_RGB,GL_UNSIGNED_BYTE,&pixelcolor);

        if(pixelcolor[0]==0 && pixelcolor[1]==255 && pixelcolor[2]==255)
        {
            return(0);
        }
        else if(pixelcolor[0]==255 && pixelcolor[1]==0 && pixelcolor[2]==255)
        {
            enemydetection=1;
        }
        else if(pixelcolor[0]==255 && pixelcolor[1]==255 && pixelcolor[2]==0)
        {
            coinelimination=1;
        }
        else
        {
            return(1);
        }
        break;
        }
    case 3:// RIGHT
        {
            glReadPixels(wndCoord(h+0.01),wndCoord(k),1,1,GL_RGB,GL_UNSIGNED_BYTE,&pixelcolor);

        if(pixelcolor[0]==0 && pixelcolor[1]==255 && pixelcolor[2]==255)
        {
            return(0);
        }
        else if(pixelcolor[0]==255 && pixelcolor[1]==0 && pixelcolor[2]==255)
        {
            enemydetection=1;
        }
        else if(pixelcolor[0]==255 && pixelcolor[1]==255 && pixelcolor[2]==0)
        {
            coinelimination=1;
        }
        else
        {
            return(1);
        }
        break;
        }
    case 4:// LEFT
        {
            glReadPixels(wndCoord(h-0.01),wndCoord(k),1,1,GL_RGB,GL_UNSIGNED_BYTE,&pixelcolor);

        if(pixelcolor[0]==0 && pixelcolor[1]==255 && pixelcolor[2]==255)
        {
            return(0);
        }
        else if(pixelcolor[0]==255 && pixelcolor[1]==0 && pixelcolor[2]==255)
        {
            enemydetection=1;
        }
        else if(pixelcolor[0]==255 && pixelcolor[1]==255 && pixelcolor[2]==0)
        {
            coinelimination=1;
        }
        else
        {
            return(1);
        }
        break;
        }
     }
}

void EnemyDetection(float h,float k)
{
    GLubyte pixelcolor1[3];
    GLubyte pixelcolor2[3];
    GLubyte pixelcolor3[3];
    GLubyte pixelcolor4[3];


    glReadBuffer(GL_BACK);

    //Is Thoplo above the enemy ?
    glReadPixels(wndCoord(h),wndCoord(k+0.01),1,1,GL_RGB,GL_UNSIGNED_BYTE,&pixelcolor1);

         if(pixelcolor1[0]==255 && pixelcolor1[1]==0 && pixelcolor1[2]==255)
        {
            enemydetection=1;
        }

    //Is Thoplo below the enemy ?
    glReadPixels(wndCoord(h),wndCoord(k-0.01),1,1,GL_RGB,GL_UNSIGNED_BYTE,&pixelcolor2);

         if(pixelcolor2[0]==255 && pixelcolor2[1]==0 && pixelcolor2[2]==255)
        {
            enemydetection=1;
        }

    //Is Thoplo to the left of the enemy ?
    glReadPixels(wndCoord(h-0.01),wndCoord(k),1,1,GL_RGB,GL_UNSIGNED_BYTE,&pixelcolor3);

         if(pixelcolor3[0]==255 && pixelcolor3[1]==0 && pixelcolor3[2]==255)
        {
            enemydetection=1;
        }
    //Is Thoplo to the right of the enemy ?
    glReadPixels(wndCoord(h+0.01),wndCoord(k),1,1,GL_RGB,GL_UNSIGNED_BYTE,&pixelcolor4);

         if(pixelcolor4[0]==255 && pixelcolor4[1]==0 && pixelcolor4[2]==255)
        {
            enemydetection=1;
        }


}

void CheckGameOver(int Level)
{
    if(ThoploLives<0)
    {
        Resetcoordinates(&h,&k);

        if(Level==1) ResetCoins(1);
        else if(Level==2) ResetCoins(2);

        score.GameOverScore=score.totalPoints;
        ResetScore();
        Sleep(500);
        ThoploLives=3;
        game_over=1;
        game_over_count=0;
        score.enemyCollision=0;// This is done so as to reset the score to 0 and prevent further deduction of score before gameplay.
    }

}


int LevelCompletion(float h, float k)
{
    if(h>1.9 && k<0.15)
    {
        RunTimeAction="Level Completed !!!";

        tempDC=GetDC(Window);
        SetBkColor(tempDC,RGB(0,0,0));
        TextOut(tempDC,700,500,RunTimeAction,strlen(RunTimeAction));
        ReleaseDC(Window,tempDC);
        menuentry=0;
        return 1;
    }
    else
    {
        RunTimeAction="                             " ;

        tempDC=GetDC(Window);
        SetBkColor(tempDC,RGB(0,0,0));
        TextOut(tempDC,700,400,RunTimeAction,strlen(RunTimeAction));

        ReleaseDC(Window,tempDC);

        return 0;

    }
}

int CheckHighScores(int ObtainedScore)
{
    int i=0,j=0,count=0,leaderboard=0;

    struct highscoreInfo *scorer; // One that holds all the score of all the user ever
    struct highscoreInfo receiver; // One that retrieves score info of a particular user from file
    struct highscoreInfo temp; // One that is used for swapping in sorting

    HighScores=fopen("D:\\Projects\\1st Sem\\Thoplo Game\\Scores.txt","a+b");
    if(HighScores==NULL)
    {
        HighScores=fopen("D:\\ThoploScores.txt","a+b");
    }

    fseek(HighScores,0,SEEK_SET);

    while(fread(&receiver,sizeof(receiver),1,HighScores)==1)
    {
        count++;
    }

    scorer=(struct highscoreInfo*)malloc(count*sizeof(struct highscoreInfo));

    fseek(HighScores,0,SEEK_SET);

    while(fread(&receiver,sizeof(receiver),1,HighScores)==1)
    {
        scorer[i].score=receiver.score;
        strcpy(scorer[i].name,receiver.name);
        i++;
    }

    for(i=0;i<count-1;i++)
    {
        for(j=i;j<count;j++)
        {
            if(scorer[i].score<scorer[j].score)
            {
                temp=scorer[j];
                scorer[j]=scorer[i];
                scorer[i]=temp;
            }
        }
    }
    fclose(HighScores);

    if(ObtainedScore>=scorer[0].score)
    {
        return 2;
    }
    else
    {
        for(i=0;i<8;i++)
            {
                if(ObtainedScore>=scorer[i].score)
                    leaderboard=1;
            }
        if(leaderboard==1) return 1;
        else if(leaderboard==0) return 0;
    }

}

void ResetScore()
{
    score.totalPoints=0;
}

void ResetCoins(int Level)
{
    switch(Level)
    {
    case 1:
        {
           l1coinx[0]=1.84;l1coinx[1]=0.15;l1coinx[2]=0.65;l1coinx[3]=1.84;l1coinx[4]=0.15;l1coinx[5]=0.4;
           l1coiny[0]=1.65;l1coiny[1]=1.65;l1coiny[2]=1.25;l1coiny[3]=0.75;l1coiny[4]=0.85;l1coiny[5]=0.15;
           break;
        }
    case 2:
        {
           l2coinx[0]=0.15;l2coinx[1]=0.25;l2coinx[2]=1.05;l2coinx[3]=1.45;l2coinx[4]=0.15;l2coinx[5]=0.15;
           l2coiny[0]=1.65;l2coiny[1]=1.45;l2coiny[2]=1.25;l2coiny[3]=0.75;l2coiny[4]=0.45;l2coiny[5]=0.15;
           break;
        }
    }
}

void BringtheCoins(int Level)
{
    int coin_to_be_eliminated=-1;
    int i=0;

    glEnable(GL_POINT_SMOOTH);

    glColor3d(255,255,0);
    glPointSize(14.0);

    switch(Level)
    {
     case 1:
     {
             // Check if the coin is eaten or not to eliminate the eaten coin.
        if(coinelimination==1)
        {
            score.coinPoints=1;
            CoinDetectionText();
            coin_to_be_eliminated=EliminateCoin(h,k,l1coinx,l1coiny);
            coinelimination=0;

            for(i=0;i<10;i++)
            {
              if(coin_to_be_eliminated==i) // The changed coin co-ordinates have to be set to default
              {
                l1coinx[i]=-1.0;
                l1coiny[i]=-1.0;
              }
            }
         }

         glBegin(GL_POINTS);

                 glVertex2f(l1coinx[0],l1coiny[0]);
                 glVertex2f(l1coinx[1],l1coiny[1]);
                 glVertex2f(l1coinx[2],l1coiny[2]);
                 glVertex2f(l1coinx[3],l1coiny[3]);
                 glVertex2f(l1coinx[4],l1coiny[4]);
                 glVertex2f(l1coinx[5],l1coiny[5]);

         glEnd();
         break;
       }
      case 2:
        {
                     // Check if the coin is eaten or not to eliminate the eaten coin.
        if(coinelimination==1)
        {
            score.coinPoints=1;
            CoinDetectionText();
            coin_to_be_eliminated=EliminateCoin(h,k,l2coinx,l2coiny);
            coinelimination=0;

            for(i=0;i<10;i++)
            {
              if(coin_to_be_eliminated==i) // The changed coin co-ordinates have to be set to default
              {
                l2coinx[i]=-1.0;
                l2coiny[i]=-1.0;
              }
            }
         }

         glBegin(GL_POINTS);

                 glVertex2f(l2coinx[0],l2coiny[0]);
                 glVertex2f(l2coinx[1],l2coiny[1]);
                 glVertex2f(l2coinx[2],l2coiny[2]);
                 glVertex2f(l2coinx[3],l2coiny[3]);
                 glVertex2f(l2coinx[4],l2coiny[4]);
                 glVertex2f(l2coinx[5],l2coiny[5]);

         glEnd();
         break;

        }
    }

}

void BringtheEnemies(int Level)
{
    static float l1enemyx[5]={0.67,0.87,0.25,1.75,0.14},l1enemyy[5]={1.64,1.24,0.83,0.76,0.35},l2enemyx[6]={0.24,0.14,0.24,1.85,1.05,1.04},l2enemyy[6]={1.64,0.84,1.04,0.84,0.44,0.2};
    static float l1enemyx_dir[5]={-1,-1,1,0,-1},l1enemyy_dir[5]={1,1,-1,-1,0}; // 1 for positive direction, 0 for negative direction, -1 for constant
    static float l2enemyx_dir[6]={1,-1,1,-1,-1,1},l2enemyy_dir[6]={-1,1,-1,1,1,-1};

    glDisable(GL_POINT_SMOOTH);

    glColor3d(255,0,255);
    glPointSize(19.0);

    switch(Level)
    {
     case 1:
         {
             glBegin(GL_POINTS);

                 glVertex2f(l1enemyx[0],l1enemyy[0]);
                 glVertex2f(l1enemyx[1],l1enemyy[1]);
                 glVertex2f(l1enemyx[2],l1enemyy[2]);
                 glVertex2f(l1enemyx[3],l1enemyy[3]);
                 glVertex2f(l1enemyx[4],l1enemyy[4]);

             glEnd();

         if(!display_certificate)
            {

             //Moving enemy1
             if(l1enemyy_dir[0]==1 && l1enemyy[0]<=1.85)
                l1enemyy[0]=l1enemyy[0]+0.001;
             if(l1enemyy[0]>=1.85||l1enemyy[0]<=1.64)
                l1enemyy_dir[0]=!l1enemyy_dir[0];
             if(l1enemyy_dir[0]==0 && l1enemyy[0]>=1.64)
                l1enemyy[0]=l1enemyy[0]-0.001;

             //Moving enemy2
             if(l1enemyy_dir[1]==1 && l1enemyy[1]<=1.45)
                l1enemyy[1]=l1enemyy[1]+0.001;
             if(l1enemyy[1]>=1.45||l1enemyy[1]<=1.24)
                l1enemyy_dir[1]=!l1enemyy_dir[1];
             if(l1enemyy_dir[1]==0 && l1enemyy[1]>=1.24)
                l1enemyy[1]=l1enemyy[1]-0.001;

             //Moving enemy3
             if(l1enemyx_dir[2]==1 && l1enemyx[2]<=1.75)
                l1enemyx[2]=l1enemyx[2]+0.0013;
             if(l1enemyx[2]>=1.75||l1enemyx[2]<=0.25)
                l1enemyx_dir[2]=!l1enemyx_dir[2];
             if(l1enemyx_dir[2]==0 && l1enemyx[2]>=0.25)
                l1enemyx[2]=l1enemyx[2]-0.0013;

             //Moving enemy4
             if(l1enemyx_dir[3]==0 && l1enemyx[3]>=0.25)
                l1enemyx[3]=l1enemyx[3]-0.0013;
             if(l1enemyx[3]>=1.75||l1enemyx[3]<=0.25)
                l1enemyx_dir[3]=!l1enemyx_dir[3];
             if(l1enemyx_dir[3]==1 && l1enemyx[3]<=1.75)
                l1enemyx[3]=l1enemyx[3]+0.0013;

             //Moving enemy5
             if(l1enemyy_dir[4]==1 && l1enemyy[4]<=0.55)
                l1enemyy[4]=l1enemyy[4]+0.0012;
             if(l1enemyy[4]>=0.55||l1enemyy[4]<=0.04)
                l1enemyy_dir[4]=!l1enemyy_dir[4];
             if(l1enemyy_dir[4]==0 && l1enemyy[4]>=0.04)
                l1enemyy[4]=l1enemyy[4]-0.0012;
            }
            break;

         }
     case 2:
        {
            glBegin(GL_POINTS);
                 glVertex2f(l2enemyx[0],l2enemyy[0]);
                 glVertex2f(l2enemyx[1],l2enemyy[1]);
                 glVertex2f(l2enemyx[2],l2enemyy[2]);
                 glVertex2f(l2enemyx[3],l2enemyy[3]);
                 glVertex2f(l2enemyx[4],l2enemyy[4]);
             glEnd();
             glPointSize(24);
             glBegin(GL_POINTS);
                glVertex2f(l2enemyx[5],l2enemyy[5]);
             glEnd();

             if(!display_certificate)
            {

             //Moving enemy1
             if(l2enemyx_dir[0]==1 && l2enemyx[0]<=1.74)
                l2enemyx[0]=l2enemyx[0]+0.00135;
             if(l2enemyx[0]>=1.74||l2enemyx[0]<=0.24)
                l2enemyx_dir[0]=!l2enemyx_dir[0];
             if(l2enemyx_dir[0]==0 && l2enemyx[0]>=0.24)
                l2enemyx[0]=l2enemyx[0]-0.00135;

             //Moving enemy2
             if(l2enemyy_dir[1]==1 && l2enemyy[1]<=1.45)
                l2enemyy[1]=l2enemyy[1]+0.00135;
             if(l2enemyy[1]>=1.45||l2enemyy[1]<=0.84)
                l2enemyy_dir[1]=!l2enemyy_dir[1];
             if(l2enemyy_dir[1]==0 && l2enemyy[1]>=0.84)
                l2enemyy[1]=l2enemyy[1]-0.00135;

             //Moving enemy3
             if(l2enemyx_dir[2]==1 && l2enemyx[2]<=1.75)
                l2enemyx[2]=l2enemyx[2]+0.00155;
             if(l2enemyx[2]>=1.75||l2enemyx[2]<=0.24)
                l2enemyx_dir[2]=!l2enemyx_dir[2];
             if(l2enemyx_dir[2]==0 && l2enemyx[2]>=0.24)
                l2enemyx[2]=l2enemyx[2]-0.00155;

             //Moving enemy4
             if(l2enemyy_dir[3]==1 && l2enemyy[3]<=1.45)
                l2enemyy[3]=l2enemyy[3]+0.00155;
             if(l2enemyy[3]>=1.45||l2enemyy[3]<=0.84)
                l2enemyy_dir[3]=!l2enemyy_dir[3];
             if(l2enemyy_dir[3]==0 && l2enemyy[3]>=0.84)
                l2enemyy[3]=l2enemyy[3]-0.00155;

             //Moving enemy5
             if(l2enemyy_dir[4]==1 && l2enemyy[4]<=0.65)
                l2enemyy[4]=l2enemyy[4]+0.0014;
             if(l2enemyy[4]>=0.65||l2enemyy[4]<=0.44)
                l2enemyy_dir[4]=!l2enemyy_dir[4];
             if(l2enemyy_dir[4]==0 && l2enemyy[4]>=0.44)
                l2enemyy[4]=l2enemyy[4]-0.0014;

             //Moving enemy6
             if(l2enemyx_dir[5]==1 && l2enemyx[5]<=1.75)
                l2enemyx[5]=l2enemyx[5]+0.0015;
             if(l2enemyx[5]>=1.75||l2enemyx[5]<=1.04)
                l2enemyx_dir[5]=!l2enemyx_dir[5];
             if(l2enemyx_dir[5]==0 && l2enemyx[5]>=1.04)
                l2enemyx[5]=l2enemyx[5]-0.0015;

            }
            break;
        }
    }
}

void BringtheDot(float x,float y) // Bring the thoplo in desired location
{
    glEnable(GL_POINT_SMOOTH);


    glColor3d(255,0,0);
    glPointSize(17.0);

    glBegin(GL_POINTS);
        glVertex2f(x,y);
    glEnd();

}

void MovetheDot(float *ph, float *pk)// To check the movement flag and move the dot as input
{

    switch(movement)
    {
    case 1:
        {
            *pk+=0.05;

            if(!MoveValidity(*ph,*pk,1))
            {
                *pk-=0.05;
            }

            break;
        }
    case 2:
        {
            *pk-=0.05;

            if(!MoveValidity(*ph,*pk,2))
            {
                *pk+=0.05;

            }

            break;
        }
    case 3:
        {
            *ph+=0.05;

            if(!MoveValidity(*ph,*pk,3))
            {
                *ph-=0.05;
            }

            break;
        }
    case 4:
        {
            *ph-=0.05;

            if(!MoveValidity(*ph,*pk,4))
            {
                *ph+=0.05;
            }

            break;
        }

    }

    movement=0;
}

void EnemyCollisionText()
{
        ThoploLives=ThoploLives-1;

        RunTimeAction="  WASTED !!!    ";
        tempDC=GetDC(Window);
        SetBkColor(tempDC,RGB(0,0,0));
        SetTextColor(tempDC,RGB(255,255,255));
        TextOut(tempDC,700,500,RunTimeAction,strlen(RunTimeAction));

        SetTextColor(tempDC,RGB(255,255,0));
        TextOut(tempDC,730,50," - 3 0 ",7);

        ReleaseDC(Window,tempDC);

        Sleep(500);

}

void CoinDetectionText()
{
        tempDC=GetDC(Window);
        SetBkColor(tempDC,RGB(0,0,0));
        SetTextColor(tempDC,RGB(255,255,0));
        TextOut(tempDC,730,50," + 5 0 ",7);
        ReleaseDC(Window,tempDC);
        Sleep(100);

}

void RenderLoading()
{
    char *LoadText=" L O A D I N G  !!!";
    static float loop=0.2;

    tempDC=GetDC(RenderingWindow);
        SetBkColor(tempDC,RGB(0,0,0));
        SetTextColor(tempDC,RGB(255,255,255));
        TextOut(tempDC,240,350,LoadText,strlen(LoadText));
    ReleaseDC(RenderingWindow,tempDC);

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0,0.0,0.0,0.0);

    if(loop<=1.7)
    {
        glBegin(GL_QUADS);
            glColor3f(0.5,0.3,0.8);
            glVertex2f(0.2,0.5);
            glVertex2f(0.2,0.6);
            glVertex2f(loop+0.1,0.6);
            glVertex2f(loop+0.1,0.5);
        glEnd();
        loop=loop+0.1;
    }
    else
    {
        menuentry=1;
        loop=0.2;
    }

    glBegin(GL_QUADS);
        glColor3f(1.0,1.0,1.0);
        glVertex2f(0.2,0.5);
        glVertex2f(0.2,0.6);
        glVertex2f(1.8,0.6);
        glVertex2f(1.8,0.5);
    glEnd();

    Sleep(100);

    glPopMatrix();

    SwapBuffers(hDC);

}

void RenderHighScores()
{

    glClearColor(0.0,0.0,0.0,0.0);

    int i=0,j=0,count=0;
    char scorenum[20]; // temp string to print score of the retrieved user

    struct highscoreInfo *scorer; // One that holds all the score of all the user ever
    struct highscoreInfo receiver; // One that retrieves score info of a particular user from file
    struct highscoreInfo temp; // One that is used for swapping in sorting

    HighScores=fopen("D:\\Projects\\1st Sem\\Thoplo Game\\Scores.txt","a+b");
    if(HighScores==NULL)
    {
        HighScores=fopen("D:\\ThoploScores.txt","a+b");
    }

    fseek(HighScores,0,SEEK_SET);

    while(fread(&receiver,sizeof(receiver),1,HighScores)==1)
    {
        count++;
    }

    scorer=(struct highscoreInfo*)malloc(count*sizeof(struct highscoreInfo));

    fseek(HighScores,0,SEEK_SET);

    while(fread(&receiver,sizeof(receiver),1,HighScores)==1)
    {
        scorer[i].score=receiver.score;
        strcpy(scorer[i].name,receiver.name);
        i++;
    }

    for(i=0;i<count-1;i++)
    {
        for(j=i;j<count;j++)
        {
            if(scorer[i].score<scorer[j].score)
            {
                temp=scorer[j];
                scorer[j]=scorer[i];
                scorer[i]=temp;
            }
        }
    }

    fclose(HighScores);


    tempDC=GetDC(RenderingWindow);
        SetBkColor(tempDC,RGB(0,0,0));

        // Intro Part
        SetTextColor(tempDC,RGB(255,255,0));
        TextOut(tempDC,170,20,"                  H I G H   S C O R E S           ",50);
        SetTextColor(tempDC,RGB(200,50,0));
        TextOut(tempDC,95,60,"    FOLLOWING IS THE LIST OF THE HIGHEST SCORES ACHIEVED        ",60);

        SetTextColor(tempDC,RGB(0,255,255));
        TextOut(tempDC,175,130," S C O R E S ",13);
        TextOut(tempDC,310,130," P L A Y E R S",14);

        SetTextColor(tempDC,RGB(255,255,255));

        for(i=0;i<8;i++)
        {
            sprintf(scorenum,"%d.         %d ",i+1,scorer[i].score);
            TextOut(tempDC,180,170+i*20,scorenum,strlen(scorenum));
            TextOut(tempDC,320,170+i*20,scorer[i].name,strlen(scorer[i].name));
        }

        ReleaseDC(RenderingWindow,tempDC);

    tempDC=GetDC(Window);
        SetBkColor(tempDC,RGB(0,0,0));
        TextOut(tempDC,690,100,"                                   ",32);
        TextOut(tempDC,690,120,"                                   ",32);

    ReleaseDC(Window,tempDC);

    Sleep(30);

    glPopMatrix();
    SwapBuffers(hDC);

}

void RenderInstructions()
{
    glClearColor(0.0,0.0,0.0,0.0);

    tempDC=GetDC(RenderingWindow);
        SetBkColor(tempDC,RGB(0,0,0));

        // Intro Part
        SetTextColor(tempDC,RGB(255,255,0));
        TextOut(tempDC,170,20,"T H O P L O ,  T H E   G A M E  v 1.0            ",47);
        SetTextColor(tempDC,RGB(200,50,0));
        TextOut(tempDC,95,60,"    IN-GAME  CONTROLS ,  OBJECTIVES  AND  SCORING  SCHEMES        ",60);
        SetTextColor(tempDC,RGB(0,255,255));
        TextOut(tempDC,130,85,"   A SINGLE PLAYER, MULTI USER, ADVENTURE GAME   ",46);

        // Developer's Name
        SetTextColor(tempDC,RGB(255,255,255));
        TextOut(tempDC,40,150,"  »   Use the Arrow Keys to move the Thoplo along the game field ",70);
        TextOut(tempDC,40,175,"  »   The moving squares are your enemies, they have taken over your planet.",75);
        TextOut(tempDC,40,200,"  »   Avoid them in your journey to the escape. Each collision deducts  30 pts.",79);
        TextOut(tempDC,40,225,"  »   The yellow thoplos are your potions. Feed on them to rejuvinate.",70);
        TextOut(tempDC,40,250,"  »   Each potion adds 50 pts to your account.",50);
        TextOut(tempDC,40,275,"  »   You are given 3 lives to complete the escape. Each collision deducts 1 life",81);




        // Preface Text
        SetTextColor(tempDC,RGB(200,100,100));
        TextOut(tempDC,100,400,"  You can play either as a guest or a Logged In User",52);
        TextOut(tempDC,50,420,"In Guest mode, you are a random user and your scores aren't tracked upon. ",74);
        TextOut(tempDC,50,440,"But as a Logged In User, you can keep track of your scores in Thoplo. ",75);


    ReleaseDC(RenderingWindow,tempDC);

    tempDC=GetDC(Window);
        SetBkColor(tempDC,RGB(0,0,0));
        TextOut(tempDC,690,100,"                                   ",32);
        TextOut(tempDC,690,120,"                                   ",32);

    ReleaseDC(Window,tempDC);

    Sleep(30);

    glPopMatrix();
    SwapBuffers(hDC);

}

void RenderDevInfo()
{
    glClearColor(0.0,0.0,0.0,0.0);

    tempDC=GetDC(RenderingWindow);
        SetBkColor(tempDC,RGB(0,0,0));

        // Intro Part
        SetTextColor(tempDC,RGB(255,255,0));
        TextOut(tempDC,170,20,"     T H O P L O ,  T H E   G A M E  v 1.0           ",50);
        SetTextColor(tempDC,RGB(200,50,0));
        TextOut(tempDC,95,60,"    ©  PD3   TECHNOLOGIES. INC  ,  ALL RIGHTS RESERVED       ",60);
        SetTextColor(tempDC,RGB(0,255,255));
        TextOut(tempDC,170,85,"   IOE   BCT-072 ,  PULCHOWK  , KTM              ",40);

        // Developer's Name
        SetTextColor(tempDC,RGB(255,255,255));
        TextOut(tempDC,170,170,"  »   PAUDEL,  ASHISH      ( BCT - 506 ) ",40);
        TextOut(tempDC,170,195,"  »   DOTEL,  SARAMSHA     ( BCT - 534 ) ",40);
        TextOut(tempDC,170,220,"  »   DAHAL,  SIMON        ( BCT - 538 )",40);
        TextOut(tempDC,170,245,"  »   DHUNGANA,  SUMIT     ( BCT - 543 ) ",40);

        // Preface Text
        SetTextColor(tempDC,RGB(200,100,100));
        TextOut(tempDC,50,400,"  This copy of the Software is the production of PD3 Technologies. Inc.",70);
        TextOut(tempDC,50,420,"Any attempt to duplicate or reverse-engineer the product is illegal. Any action as ",80);
        TextOut(tempDC,20,440,"such will be prosecuted under legal jurisdiction in accordance to the Copyright act  ",85);
        TextOut(tempDC,25,460,"of Constitution of Nepal-2072. The name PD3 Technologies and \"Thoplo\" , and the ",80);
        TextOut(tempDC,50,480,"             logo are the Registered Trademark of the company. ",70);


    ReleaseDC(RenderingWindow,tempDC);

    tempDC=GetDC(Window);
        SetBkColor(tempDC,RGB(0,0,0));
        TextOut(tempDC,690,100,"                                   ",32);
        TextOut(tempDC,690,120,"                                   ",32);

    ReleaseDC(Window,tempDC);

    Sleep(30);

    glPopMatrix();
    SwapBuffers(hDC);

}

void RenderGameOver()
{
    int recordcount=0;
    char FinalScore[50];
    struct highscoreInfo newscore;
    struct userInfo currentUser;

    glClearColor(0.0,0.0,0.0,0.0);

    newscore.score=score.GameOverScore;
    strcpy(newscore.name,PlayerName);

    tempDC=GetDC(RenderingWindow);
    SetBkColor(tempDC,RGB(0,0,0));

    switch(CheckHighScores(score.GameOverScore))
    {
    case 0:
        {
           SetTextColor(tempDC,RGB(200,100,0));
           TextOut(tempDC,100,100,"                                                                                 ",55);
            break;
        }
    case 1:
        {
            SetTextColor(tempDC,RGB(200,100,0));
            TextOut(tempDC,100,100,"   C O N G R A T S  !!! ,  YOU MADE IT INTO THE LEADERBOARDS     ",60);
            break;
        }
    case 2:
        {
            SetTextColor(tempDC,RGB(200,100,0));
            TextOut(tempDC,100,100,"   B R A V O   !!! ,   YOU MADE THE HIGHEST SCORE     ",55);
            break;
        }
    }

    if(game_over_count==0)
    {
        // Editing the individual HighScore of the User (conditional if the score is above his previous highscore)
        User=fopen("D:\\Projects\\1st Sem\\Thoplo Game\\Users.txt","r+b");
                if(User==NULL)
                    {
                        User=fopen("D:\\ThoploUsers.txt","r+b");
                    }
                fseek(User,0,SEEK_SET);

                while(fread(&currentUser,sizeof(currentUser),1,User)==1)
                {
                    if((strcmp(PlayerName,currentUser.name)==0))
                       {
                            if(newscore.score >= currentUser.highscore)
                            {
                                    sprintf(CurrentUserScore,"CONGRATS !!! This is your Individual High Score ");
                                    currentUser.highscore=newscore.score;
                                    fseek(User,sizeof(currentUser)*recordcount,SEEK_SET);
                                    fwrite(&currentUser,sizeof(currentUser),1,User);
                                    break;
                            }
                            else
                            {
                                sprintf(CurrentUserScore," Your all Time High Score in Thoplo is %d ",currentUser.highscore);
                            }
                       }
                       recordcount++;
                }

                fclose(User);

        //Adding the new Score-User info
        HighScores=fopen("D:\\Projects\\1st Sem\\Thoplo Game\\Scores.txt","a+b");
        if(HighScores==NULL)
        {
            HighScores=fopen("D:\\ThoploScores.txt","a+b");
        }

        fseek(HighScores,0,SEEK_END);
        fwrite(&newscore,sizeof(newscore),1,HighScores);
        fclose(HighScores);
        game_over_count=1;
    }
        SetTextColor(tempDC,RGB(255,255,255));
        TextOut(tempDC,150,450,CurrentUserScore,strlen(CurrentUserScore));
        SetTextColor(tempDC,RGB(255,255,0));
        TextOut(tempDC,190,280,"      G A M E    O V E R   !!!     ",35);
        sprintf(FinalScore,"       FINAL  SCORE  :  %d",score.GameOverScore);
        SetTextColor(tempDC,RGB(0,255,255));
        TextOut(tempDC,190,310,FinalScore,strlen(FinalScore));

    ReleaseDC(RenderingWindow,tempDC);

    Sleep(20);

    tempDC=GetDC(Window);
        SetBkColor(tempDC,RGB(0,0,0));
        TextOut(tempDC,690,100,"                                   ",32);
        TextOut(tempDC,690,120,"                                   ",32);
        TextOut(tempDC,0,100,"                                   ",31);
        TextOut(tempDC,0,120,"                                   ",31);
    ReleaseDC(Window,tempDC);

    glPopMatrix();
    SwapBuffers(hDC);

}


void RenderStatic()// The primitive rendering or the introduction box
{
    tempDC=GetDC(RenderingWindow);
        SetBkColor(tempDC,RGB(0,0,0));
        SetTextColor(tempDC,RGB(255,255,255));
        TextOut(tempDC,449,280,"      THE GAME     ",19);
    ReleaseDC(RenderingWindow,tempDC);

    glClearColor(0.5,0.7,0.7,0.4);

    glPointSize(18.0);
    glEnable(GL_POINT_SMOOTH);

    glBegin(GL_POINTS);
        // T
        glColor3f(1.0,0.5,0.0);
        glVertex2f(0.2,1.4);
        glVertex2f(0.3,1.4);
        glVertex2f(0.4,1.4);
        glVertex2f(0.3,1.3);
        glVertex2f(0.3,1.2);
        glVertex2f(0.3,1.1);

        //H
        glColor3f(0.8,0.7,0.8);
        glVertex2f(0.55,1.4);
        glVertex2f(0.55,1.3);
        glVertex2f(0.55,1.2);
        glVertex2f(0.55,1.1);
        glVertex2f(0.6,1.25);
        glVertex2f(0.675,1.25);
        glVertex2f(0.72,1.4);
        glVertex2f(0.72,1.3);
        glVertex2f(0.72,1.2);
        glVertex2f(0.72,1.1);

        //O
        glColor3f(1.0,0.5,0.0);
        glVertex2f(0.84,1.3);
        glVertex2f(0.84,1.2);
        glVertex2f(0.89,1.4);
        glVertex2f(0.97,1.4);
        glVertex2f(1.01,1.3);
        glVertex2f(1.01,1.2);
        glVertex2f(0.89,1.1);
        glVertex2f(0.97,1.1);

        //P
        glColor3f(0.8,0.7,0.8);
        glVertex2f(1.13,1.4);
        glVertex2f(1.13,1.3);
        glVertex2f(1.13,1.2);
        glVertex2f(1.13,1.1);
        glVertex2f(1.2,1.4);
        glVertex2f(1.27,1.4);
        glVertex2f(1.3,1.325);
        glVertex2f(1.27,1.27);
        glVertex2f(1.2,1.27);

        //L
        glColor3f(1.0,0.5,0.0);
        glVertex2f(1.42,1.4);
        glVertex2f(1.42,1.3);
        glVertex2f(1.42,1.2);
        glVertex2f(1.42,1.1);
        glVertex2f(1.5,1.1);
        glVertex2f(1.585,1.1);

        //O
        glColor3f(0.8,0.7,0.8);
        glVertex2f(1.67,1.3);
        glVertex2f(1.67,1.2);
        glVertex2f(1.72,1.4);
        glVertex2f(1.8,1.4);
        glVertex2f(1.85,1.3);
        glVertex2f(1.85,1.2);
        glVertex2f(1.72,1.1);
        glVertex2f(1.8,1.1);

    glEnd();


    glBegin(GL_QUADS);
        glColor3f(0.0,0.0,0.0);
        glVertex2f(0.0,1.0);
        glVertex2f(0.0,1.5);
        glVertex2f(2.0,1.5);
        glVertex2f(2.0,1.0);
    glEnd();

    Sleep(10);

    glPopMatrix();

    SwapBuffers(hDC);

}

void RenderLevel1()// To Render the Layout of Game
{
    glClearColor(0.0,0.0,0.0,0.0);

    float i;
    float j;


    glEnable(GL_LINE_SMOOTH);

    glBegin(GL_QUADS);

       glColor3f(0.0,1.0,1.0);

       glVertex2f(0.0,1.8);
       glVertex2f(0.0,1.89);
       glVertex2f(0.06,1.89);
       glVertex2f(0.06,1.8);

        //quad1
       glVertex2f(0.0,1.89);
       glVertex2f(0.0,2.0);
       glVertex2f(2.0,2.0);
       glVertex2f(2.0,1.89);
       //quad2
       glVertex2f(0.0,1.69);
       glVertex2f(0.0,1.8);
       glVertex2f(0.62,1.8);
       glVertex2f(0.62,1.69);
       //quad3
       glVertex2f(0.71,1.69);
       glVertex2f(0.71,1.8);
       glVertex2f(1.8,1.8);
       glVertex2f(1.8,1.69);
       //quad4
       glVertex2f(1.89,1.9);
       glVertex2f(2.0,1.9);
       glVertex2f(2.0,0.2);
       glVertex2f(1.89,0.2);
       //quad5
       glVertex2f(0.1,1.49);
       glVertex2f(0.1,1.6);
       glVertex2f(1.2,1.6);
       glVertex2f(1.2,1.49);
       //quad6
       glVertex2f(1.29,1.49);
       glVertex2f(1.29,1.6);
       glVertex2f(1.9,1.6);
       glVertex2f(1.9,1.49);
       //quad7
       glVertex2f(0.0,0.0);
       glVertex2f(0.0,1.7);
       glVertex2f(0.1,1.7);
       glVertex2f(0.1,0.0);
       //quad8
       glVertex2f(0.19,1.29);
       glVertex2f(0.19,1.4);
       glVertex2f(0.82,1.4);
       glVertex2f(0.82,1.29);
       //quad9
       glVertex2f(0.91,1.29);
       glVertex2f(0.91,1.4);
       glVertex2f(1.9,1.4);
       glVertex2f(1.9,1.29);
       //quad10
       glVertex2f(0.1,1.09);
       glVertex2f(0.1,1.2);
       glVertex2f(1.8,1.2);
       glVertex2f(1.8,1.09);
       //quad11
       glVertex2f(0.19,0.89);
       glVertex2f(0.19,1.0);
       glVertex2f(1.0,1.0);
       glVertex2f(1.0,0.89);
       //quad12
       glVertex2f(1.09,0.89);
       glVertex2f(1.09,1.0);
       glVertex2f(1.9,1.0);
       glVertex2f(1.9,0.89);
       //quad13
       glVertex2f(0.1,0.59);
       glVertex2f(0.1,0.7);
       glVertex2f(1.8,0.7);
       glVertex2f(1.8,0.59);
       //quad14
       glVertex2f(0.18,0.39);
       glVertex2f(0.18,0.5);
       glVertex2f(1.9,0.5);
       glVertex2f(1.9,0.39);
       //quad15
       glVertex2f(0.18,0.19);
       glVertex2f(0.18,0.31);
       glVertex2f(1.8,0.31);
       glVertex2f(1.8,0.19);
       //quad16
       glVertex2f(0.19,0.0);
       glVertex2f(0.19,0.11);
       glVertex2f(1.9,0.11);
       glVertex2f(1.9,0.0);
       //exit point/ finish line

       glColor3f(1.0,0.5,0.0);

       glVertex2f(1.8,0.0);
       glVertex2f(1.8,0.05);
       glVertex2f(2.0,0.05);
       glVertex2f(2.0,0.0);

    glEnd();

    BringtheEnemies(1); // i.e Enemies for Level 1

    BringtheCoins(1); // i.e Coins for Level 1

    if(!display_certificate) MovetheDot(&h,&k);

    BringtheDot(h,k);

    EnemyDetection(h,k);

    if(enemydetection==1)
    {
        Resetcoordinates(&h,&k);
        score.enemyCollision=1;
        EnemyCollisionText();
        enemydetection=0;
    }

    if(LevelCompletion(h,k))
    {
        Resetcoordinates(&h,&k);
        presentLevel=2;
        ResetCoins(1);
        Sleep(500);
    }

    CheckGameOver(1); // the argument being the level which Thoplo is in

    glPopMatrix();

    SwapBuffers(hDC);
}

void RenderLevel2()
{
    glClearColor(0.0,0.0,0.0,0.0);

    glEnable(GL_LINE_SMOOTH);

    glBegin(GL_QUADS);

       glColor3f(0.0,1.0,1.0);

       glVertex2f(0.0,1.8);
       glVertex2f(0.0,1.89);
       glVertex2f(0.06,1.89);
       glVertex2f(0.06,1.8);

        //quad1
       glVertex2f(0.0,1.89);
       glVertex2f(0.0,2.0);
       glVertex2f(2.0,2.0);
       glVertex2f(2.0,1.89);
       //quad2
       glVertex2f(0.0,1.69);
       glVertex2f(0.0,1.8);
       glVertex2f(1.8,1.8);
       glVertex2f(1.8,1.69);
       //quad3
       glVertex2f(1.19,1.49);
       glVertex2f(1.19,1.6);
       glVertex2f(1.9,1.6);
       glVertex2f(1.9,1.49);
       //quad4
       glVertex2f(1.89,1.9);
       glVertex2f(2.0,1.9);
       glVertex2f(2.0,0.2);
       glVertex2f(1.89,0.2);
       //quad5
       glVertex2f(0.1,1.49);
       glVertex2f(0.1,1.6);
       glVertex2f(1.1,1.6);
       glVertex2f(1.1,1.49);
       //quad6
       glVertex2f(0.19,1.29);
       glVertex2f(0.19,1.4);
       glVertex2f(1.8,1.4);
       glVertex2f(1.8,1.29);
       //quad7
       glVertex2f(0.0,0.0);
       glVertex2f(0.0,1.7);
       glVertex2f(0.1,1.7);
       glVertex2f(0.1,0.0);
       //quad8
       glVertex2f(0.19,1.09);
       glVertex2f(0.19,1.2);
       glVertex2f(1.0,1.2);
       glVertex2f(1.0,1.09);
       //quad9
       glVertex2f(1.19,1.09);
       glVertex2f(1.19,1.2);
       glVertex2f(1.8,1.2);
       glVertex2f(1.8,1.09);
       //quad10
       glVertex2f(0.19,0.89);
       glVertex2f(0.19,1.0);
       glVertex2f(1.8,1.0);
       glVertex2f(1.8,0.89);
       //quad11
       glVertex2f(0.1,0.69);
       glVertex2f(0.1,0.8);
       glVertex2f(1.4,0.8);
       glVertex2f(1.4,0.69);
       //quad12
       glVertex2f(1.49,0.69);
       glVertex2f(1.49,0.8);
       glVertex2f(1.9,0.8);
       glVertex2f(1.9,0.69);
       //quad13
       glVertex2f(0.19,0.49);
       glVertex2f(0.19,0.6);
       glVertex2f(1.0,0.6);
       glVertex2f(1.0,0.49);
       //quad14
       glVertex2f(1.09,0.49);
       glVertex2f(1.09,0.6);
       glVertex2f(1.9,0.6);
       glVertex2f(1.9,0.49);
       //quad15
       glVertex2f(0.1,0.29);
       glVertex2f(0.1,0.4);
       glVertex2f(1.4,0.4);
       glVertex2f(1.4,0.29);
       //quad16
       glVertex2f(1.49,0.29);
       glVertex2f(1.49,0.4);
       glVertex2f(1.9,0.4);
       glVertex2f(1.9,0.29);
       //quad17
       glVertex2f(0.1,0.0);
       glVertex2f(0.1,0.11);
       glVertex2f(1.9,0.11);
       glVertex2f(1.9,0.0);

       //exit point/ finish line

       glColor3f(1.0,0.5,0.0);

       glVertex2f(1.9,0.0);
       glVertex2f(1.9,0.05);
       glVertex2f(2.0,0.05);
       glVertex2f(2.0,0.0);

    glEnd();

    BringtheEnemies(2); // i.e Enemies for Level 1

    BringtheCoins(2); // i.e Coins for Level 1

    if(!display_certificate) MovetheDot(&h,&k);

    BringtheDot(h,k);

    EnemyDetection(h,k);

    if(enemydetection==1)
    {
        Resetcoordinates(&h,&k);
        score.enemyCollision=1;
        EnemyCollisionText();
        enemydetection=0;
    }

    if(LevelCompletion(h,k))
    {
        Resetcoordinates(&h,&k);
        presentLevel=1;
        ResetCoins(2);
        Sleep(500);
    }

    CheckGameOver(2); // the argument being the level which Thoplo is in


    glPopMatrix();

    SwapBuffers(hDC);
}

void ScoreCalculation()
{
    char scoreDisplay[50];


    if(score.enemyCollision==1)
    {
        score.totalPoints=score.totalPoints-30;
        score.enemyCollision=0;
    }
    if(score.coinPoints==1)
    {
        score.totalPoints=score.totalPoints+50;
        score.coinPoints=0;
    }

    sprintf(scoreDisplay,"              %d                  ",score.totalPoints);
    SetBkColor(tempDC,RGB(0,200,50));
    SetTextColor(tempDC,RGB(0,0,0));
    TextOut(tempDC,690,120,scoreDisplay,strlen(scoreDisplay));
}

void LivesCalculation()
{
    char Livechar[50];

    SetBkColor(tempDC,RGB(0,200,50));
    SetTextColor(tempDC,RGB(0,0,0));
    sprintf(Livechar,"     %d  Thoplo(s)       ",ThoploLives);
    TextOut(tempDC,0,120,Livechar,strlen(Livechar));

}

void RenderGameTimeInfo()
{
    char *PauseInfo="Press Esc to Pause";
    char LevelInfo[50];

    tempDC=GetDC(Window);
// Displaying Esc-Pause Info (conditional on display certificate)
    SetBkColor(tempDC,RGB(0,0,0));
    SetTextColor(tempDC,RGB(255,255,255));

    switch(display_certificate)
    {
        case 0:
            {
                SetBkColor(tempDC,RGB(0,0,255));
                TextOut(tempDC,690,0,PauseInfo,strlen(PauseInfo));
                sprintf(LevelInfo,"     L E V E L :     %d   ",presentLevel);
                TextOut(tempDC,0,0,LevelInfo,strlen(LevelInfo));
                TextOut(tempDC,0,17,PlayerName,strlen(PlayerName));
                break;

            }
        default:
             {
                TextOut(tempDC,690,0,"                                   ",32);
                TextOut(tempDC,0,0,"                                   ",31);
                TextOut(tempDC,0,17,"                                   ",31);
                break;
             }

    }

// Displaying Score of the Game(conditional on display certificate)

    switch(display_certificate)
    {
        case 0:
            {
            SetBkColor(tempDC,RGB(255,100,0));
            SetTextColor(tempDC,RGB(0,0,0));
            TextOut(tempDC,690,100,"        S C O R E         ",25);
            ScoreCalculation();
            break;
            }
        default:
            {
            TextOut(tempDC,690,100,"                                   ",32);
            TextOut(tempDC,690,120,"                                   ",32);
            break;
            }
    }

// Displaying Remaining Lives

    switch(display_certificate)
    {
        case 0:
            {
            SetBkColor(tempDC,RGB(255,100,0));
            SetTextColor(tempDC,RGB(0,0,0));
            TextOut(tempDC,0,100,"         L I V E S         ",26);
            LivesCalculation();
            break;
            }
        default:
            {
            break;
            }
    }

    SetBkColor(tempDC,RGB(0,0,0));
    TextOut(tempDC,700,500,RunTimeAction,strlen(RunTimeAction)); // Displays empty space in place of the conditional Run Time actions of the game.
    TextOut(tempDC,700,50,RunTimeAction,strlen(RunTimeAction)); // Displays empty space in place of the conditional score +/- actions of the game.

    ReleaseDC(Window,tempDC);

    tempDC==NULL;

}


void RenderChosenMenu(int menuchoice)// Back-end function to get to the clicked menu
{
     // Displaying/Hiding the Pause Menu Buttons.
        ShowWindow(bPlayGame,display_certificate);
        ShowWindow(bStoryMode,display_certificate);
        ShowWindow(bHighScores,display_certificate);
        ShowWindow(bAbout,display_certificate);
        ShowWindow(bInstructions,display_certificate);
        ShowWindow(bExit,display_certificate);
        ShowWindow(bReplay,game_over);
        ShowWindow(bMainMenu,game_over);


    RenderGameTimeInfo();

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0,0.0,0.0,0.0);

    glLoadIdentity();
    glPushMatrix();

    glTranslatef(-1.0f,-1.0f,0.0f);

    if(menuchoice==0)
    {
        RenderStatic();
    }
    else if(menuchoice==1)
    {
      if(game_over==0)
      {
        switch(presentLevel)
        {
        case 1:
            {

                if(menuentry==0)
                {
                    display_certificate=0;
                    RenderLoading();
                }
                else if(menuentry==1)
                {
                    RenderLevel1();
                }
                break;
            }
        case 2:
            {
                if(menuentry==0)
                {
                    display_certificate=0;
                    RenderLoading();
                }
                else if(menuentry==1)
                {
                    RenderLevel2();
                }
                break;

            }
         }
       }
       else if(game_over==1)
       {
           RenderGameOver();
       }

    }
    else if(menuchoice==3)
    {
        RenderHighScores();
    }
    else if(menuchoice==4)
    {
        RenderDevInfo();
    }
    else if(menuchoice==5)
    {
        RenderInstructions();
    }
}


LRESULT CALLBACK InfoDlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    struct userInfo newUser;
    struct userInfo retrieval;
    int datafound=0;

    switch(msg)
    {
    case WM_INITDIALOG:
        {
            SetDlgItemText(hWnd,IDD_DIALOG_NAME,"");
            SetDlgItemText(hWnd,IDD_DIALOG_PASSWORD,"");
            break;
        }
    case WM_COMMAND:
        {
            char Username[50];
            char Password[50];

            GetDlgItemText(hWnd,IDD_DIALOG_NAME,Username,50);
            GetDlgItemText(hWnd,IDD_DIALOG_PASSWORD,Password,50);

            if(wParam==IDLOGIN)
            {
                User=fopen("D:\\Projects\\1st Sem\\Thoplo Game\\Users.txt","a+b");
                if(User==NULL)
                    {
                        User=fopen("D:\\ThoploUsers.txt","a+b");
                    }
                fseek(User,0,SEEK_SET);

                while(fread(&retrieval,sizeof(retrieval),1,User)==1)
                {
                    if((strcmp(retrieval.name,Username)==0)&&(strcmp(retrieval.password,Password)==0))
                       {
                           datafound=1;
                           strcpy(PlayerName,Username); // To make the user the current player
                       }
                }

                fclose(User);

                if(datafound==0)
                {
                    MessageBox(hWnd," Entered User not Registered in Database !!! ","Invalid Entry",MB_ICONERROR);
                    menuchoice=0;
                }
                else if(datafound==1)
                {
                    menuentry=0;
                    Resetcoordinates(&h,&k);
                    Resetcoordinates(&h,&k);
                    ResetScore();
                    ResetCoins(1);
                    ResetCoins(2);
                    ThoploLives=3;
                    menuchoice=1;
                }

                EndDialog(hWnd,0);
            }

            else if(wParam==IDSIGNUP)
            {
                // Write the newly signed up player into the file.

                strcpy(newUser.name,Username);
                strcpy(newUser.password,Password);
                newUser.highscore=-120;

                // To check if the entered name for signup is already present or not

                User=fopen("D:\\Projects\\1st Sem\\Thoplo Game\\Users.txt","a+b");
                if(User==NULL)
                    {
                        User=fopen("D:\\ThoploUsers.txt","a+b");
                    }
                fseek(User,0,SEEK_SET);

                while(fread(&retrieval,sizeof(retrieval),1,User)==1)
                {
                    if((strcmp(retrieval.name,Username)==0))
                       {
                           datafound=1;
                           strcpy(PlayerName,Username); // To make the user the current player
                       }
                }

                fclose(User);

            if(datafound==1)
            {
                    MessageBox(hWnd," Entered User Name is already taken by another User ! ","Data Redundant",MB_ICONERROR);
                    menuchoice=0;
            }
            else
            {
                User=fopen("D:\\Projects\\1st Sem\\Thoplo Game\\Users.txt","a+b");
                if(User==NULL)
                    {
                        User=fopen("D:\\ThoploUsers.txt","a+b");
                    }
                fseek(User,0,SEEK_END);

                fwrite(&newUser,sizeof(newUser),1,User);

                fclose(User);

                strcpy(PlayerName,Username); // To make the user the current player

                menuentry=0;
                Resetcoordinates(&h,&k);
                Resetcoordinates(&h,&k);
                ResetScore();
                ResetCoins(1);
                ResetCoins(2);
                ThoploLives=3;
                menuchoice=1;

            }
            EndDialog(hWnd,0);
            }

            break;
        }

    }
    return (0);
}

void wmSize(HWND hWnd,UINT msg, WPARAM wParam, LPARAM lParam)
{
    RECT trect;
    GetClientRect(Window,&trect);

    startx=(trect.right-trect.left-(trect.bottom-trect.top))/2;
    starty=0;
    lengthy=trect.bottom-trect.top;
    lengthx=lengthy;

    MoveWindow(RenderingWindow,startx,starty,lengthx,lengthy,1);

    GetClientRect(RenderingWindow,&trect);
    ResizeGLWindow(trect.right-trect.left,trect.bottom-trect.top);
}

void wmCommand(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    if(lParam==(LPARAM)bPlayGame)
    {
         srand((unsigned)time(NULL));
         menuentry=0;
         Resetcoordinates(&h,&k);
         ResetScore();
         ResetCoins(1);
         ResetCoins(2);
         ThoploLives=3;
         presentLevel=1;
         menuchoice=1;
         GuestNumber=(rand()%20000)+1;
         sprintf(PlayerName,"Guest %d",GuestNumber);
         sprintf(CurrentUserScore," We have no track of your previous scores !!!",CurrentUserScore);
         MessageBox(hWnd,"Press OK for Quick Game. You will be logged in as a random Guest User !","Thoplo: Quick Game",MB_OK);

    }
    else if(lParam==(LPARAM)bStoryMode)
    {
         DialogBox(GlobalInstance,MAKEINTRESOURCE(IDD_DIALOG),NULL,(DLGPROC)InfoDlgProc);
    }
    else if(lParam==(LPARAM)bHighScores)menuchoice=3;
    else if(lParam==(LPARAM)bAbout)menuchoice=4;
    else if(lParam==(LPARAM)bInstructions)menuchoice=5;
    else if(lParam==(LPARAM)bExit)PostQuitMessage(0);
    else if(lParam==(LPARAM)bReplay)
    {
        game_over=0;
        presentLevel=1;
    }
    else if(lParam==(LPARAM)bMainMenu)
    {
        presentLevel=1;
        game_over=0;
        menuchoice=0;
        menuentry=0;
        display_certificate=1;
    }

}


LRESULT CALLBACK WndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
        case WM_DESTROY: PostQuitMessage(0);break;
        case WM_COMMAND:
            {
                wmCommand(hWnd,msg,wParam,lParam);

            }
        case WM_KEYDOWN:
            {

                switch(wParam)
                {
                    case VK_UP:
                        {
                            movement=1;
                            break;
                        }
                    case VK_DOWN:
                        {
                            movement=2;
                            break;
                        }
                    case VK_RIGHT:
                        {
                            movement=3;
                            break;
                        }
                    case VK_LEFT:
                        {
                            movement=4;
                            break;
                        }
                    case VK_ESCAPE:
                        {
                            display_certificate=!display_certificate;
                        }
                }
            }
        case WM_SIZE: wmSize(hWnd,msg,wParam,lParam);break;



    }
    return (DefWindowProc(hWnd,msg,wParam,lParam));
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevious, LPSTR lpcmdString, int cmdShow)
{

    GlobalInstance=hInstance;
    WNDCLASS  wc;
    char *ForeWord="Hello World";


    wc.cbClsExtra=0;
    wc.cbWndExtra=0;
    wc.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.hCursor=LoadCursor(NULL,IDC_ARROW);
    wc.hIcon=LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_ICON1));
    wc.hInstance=hInstance;
    wc.lpfnWndProc=WndProc;
    wc.lpszClassName="Hello";
    wc.lpszMenuName=NULL;
    wc.style=CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    display_certificate=cmdShow;

    if(!RegisterClass(&wc))
    {
        MessageBox(NULL,"Cannot register class","Error !",MB_OK);
        return (0);
    }

    Window= CreateWindow("Hello","Thoplo: The Game",WS_OVERLAPPEDWINDOW|WS_VISIBLE|WS_CLIPCHILDREN,200,50,832,624,NULL,NULL,hInstance,NULL);

    if(Window==NULL)
    {
        MessageBox(NULL,"Cannot create Window","Error!",MB_OK);
        return(0);
    }

    GetClientRect(Window,&rect);


    startx=(rect.right-rect.left-(rect.bottom-rect.top))/2;
    starty=0;
    lengthy=rect.bottom-rect.top;
    lengthx=lengthy;

    RenderingWindow= CreateWindow("STATIC",NULL,WS_CHILD|WS_VISIBLE|WS_BORDER,startx,starty,lengthx,lengthy,Window,NULL,hInstance,NULL);

    bPlayGame= CreateWindow("BUTTON","Quick Game",WS_CHILD,0,100,DEFAULT_BUTTON_WIDTH,DEFAULT_BUTTON_HEIGHT,Window,NULL,hInstance,NULL);

    bStoryMode= CreateWindow("BUTTON","Story Mode",WS_CHILD,0,160,DEFAULT_BUTTON_WIDTH,DEFAULT_BUTTON_HEIGHT,Window,NULL,hInstance,NULL);

    bHighScores= CreateWindow("BUTTON","High Scores",WS_CHILD,0,220,DEFAULT_BUTTON_WIDTH,DEFAULT_BUTTON_HEIGHT,Window,NULL,hInstance,NULL);

    bAbout= CreateWindow("BUTTON","Developers' Info",WS_CHILD,0,280,DEFAULT_BUTTON_WIDTH,DEFAULT_BUTTON_HEIGHT,Window,NULL,hInstance,NULL);

    bInstructions= CreateWindow("BUTTON","Instructions",WS_CHILD,0,340,DEFAULT_BUTTON_WIDTH,DEFAULT_BUTTON_HEIGHT,Window,NULL,hInstance,NULL);

    bExit= CreateWindow("BUTTON","Exit",WS_CHILD,0,400,DEFAULT_BUTTON_WIDTH,DEFAULT_BUTTON_HEIGHT,Window,NULL,hInstance,NULL);

    bReplay= CreateWindow("BUTTON","Replay",WS_CHILD,0,340,DEFAULT_BUTTON_WIDTH,DEFAULT_BUTTON_HEIGHT,Window,NULL,hInstance,NULL);

    bMainMenu= CreateWindow("BUTTON","Main Menu",WS_CHILD,690,340,DEFAULT_BUTTON_WIDTH,DEFAULT_BUTTON_HEIGHT,Window,NULL,hInstance,NULL);


    Menu=LoadMenu(hInstance,MAKEINTRESOURCE(IDR_MENU));
    SetMenu(Window,Menu);

    EnableOpenGL(RenderingWindow,&hDC,&glRC);

    GetClientRect(RenderingWindow,&rect);
    ResizeGLWindow(rect.right-rect.left,rect.bottom-rect.top);
    SetGLDefaults();

    while(1)
    {

        if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
        {
          if(msg.message==WM_QUIT)break;
          TranslateMessage(&msg);
          DispatchMessage(&msg);
        }

        RenderChosenMenu(menuchoice);
     }

    return 1;

    DisableOpenGL(RenderingWindow,hDC,glRC);
}




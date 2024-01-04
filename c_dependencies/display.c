#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/mman.h>
#include <string.h>
#include "mouse.c"
// #include "headers/display.h"
// #include "headers/server.h"

#define WIDTH 1920
#define HEIGHT 1080

short base_color = 0xffff;
long int screensize = 0;
short *buffer = 0;
short *fbp = 0;
int fbfd = 0;
int isDInit = 0;

struct point2d {
    int x;
    int y;
};

short getColor(int r, int g, int b) {
  unsigned short c = ((r / 8) << 11) + ((g / 4) << 5) + (b / 8);
  return c;
}

void drawLine(short color, struct point2d point1, struct point2d point2) {
    int xdiff = point2.x - point1.x;
    int ydiff = point2.y - point1.y;
    int steps = 0;
    
    if(xdiff == 0 && ydiff == 0) {
        return;
    }


    if (abs(xdiff) > abs(ydiff)) {
        steps = abs(xdiff);
    } else {
        steps = abs(ydiff);
    }
    for (int i = 0; i<steps; i++) {
        int x = point1.x + (xdiff * i) / steps;
        int y = point1.y + (ydiff * i) / steps;
        if (x<WIDTH && x>=0 && y<HEIGHT && y>=0){
            buffer[x+y*WIDTH] = color;
        }
    }
}

void drawPolygon(short color, int verticies, struct point2d* points) {
  if (verticies < 3) {
      return;
  }
  for (int i = 0; i < verticies; i++) {
      drawLine(color, points[i], points[(i+1)%verticies]);
  }
}

int findMin(int verticies, struct point2d* points) {
  int min = HEIGHT;
  for( int i=0; i<verticies; i++) {
    if (points[i].y < min) {
      min = points[i].y;
    }
  }
  return min;
}

int findMax(int verticies, struct point2d* points) {
  int max = 0;
  for( int i=0; i<verticies; i++) {
    if (points[i].y > max) {
      max = points[i].y;
    }
  }
  return max;
}

void findIntersect(struct point2d point1, struct point2d point2, int* intersects, int yVal) {
  double ydiff = point2.y-point1.y;
  if (point1.y > yVal && point2.y > yVal || point1.y < yVal && point2.y < yVal) {
    return;
  }

  if (ydiff == 0) {
    intersects[0] = point1.x;
    intersects[1] = point2.x;
    return;
  }

  int xVal = ((point2.x-point1.x)/(ydiff)) * (yVal - point1.y) + point1.x;
  // printf("%d\n", xVal);
  if (xVal < 0) {
    xVal = 0;
  }
  if (xVal > WIDTH) {
    xVal = WIDTH;
  }
  if (intersects[0] == -1) {
    intersects[0] = xVal;
    return;
  } else {
    if (xVal != intersects[0] && intersects[1] == -1) {
      intersects[1] = xVal;
    }
  }
}

void fillConvexPolygon(short color, int verticies, struct point2d* points) {
  int min = findMin(verticies, points);
  int max = findMax( verticies, points);
  
  for( int i = min+1; i<max; i++) {
    int intersects[2] = {-1, -1};
    for( int j=0; j<verticies; j++) {
      // printf("%d, %d\n", intersects[0], intersects[1]);
      findIntersect(points[j], points[(j+1)%verticies], intersects, i);
    }
    if (intersects[0] != -1 && intersects[1] != -1) {
      if (intersects[0] > intersects[1]) {
        for(int k = intersects[1]; k<=intersects[0]; k++) {
          buffer[WIDTH*i + k] = color;
        }
      } else {
        // memset(buffer + (WIDTH*i+intersects[0]), color, (intersects[1]-intersects[0]));
        for(int k = intersects[0]; k<=intersects[1]; k++) {
          buffer[WIDTH*i + k] = color;
        }
      }
    }
  }
}

void render() {
  memcpy(fbp, buffer, WIDTH*HEIGHT*2);
}

void correctPoint(struct point2d* point, int x, int y) {
  *point = (struct point2d){point->x + x, point->y + y};
}

void correct(struct point2d* points, int x, int y, int n) {
  correctPoint(&(points[n]), x, y);
}

//must be 500x1000
void transformImage(short* image, struct point2d* edges, struct point2d* transformations) {
  
}

void loop () {
  char input = ' ';
  int n = 0;
  struct point2d transformations[4] = {
    (struct point2d){0, 0},
    (struct point2d){0, 0},
    (struct point2d){0, 0},
    (struct point2d){0, 0}
  };
  struct point2d points[4] = {
    (struct point2d){WIDTH-100, (HEIGHT)-100}, 
    (struct point2d){WIDTH-1100, (HEIGHT)-100}, 
    (struct point2d){WIDTH-1100, (HEIGHT)-600}, 
    (struct point2d){WIDTH-100, (HEIGHT)-600}};
  while (input != 'q') {
    memset(buffer, 0x0000, screensize);
    fillConvexPolygon(base_color, 4, points);
    render();
    scanf("%c", &input);
    if (input == 'a') {
      correct(points, -5, 0, n);
      transformations[n].x -= 5;
    } else if (input == 'd') {
      correct(points, 5, 0, n);
      transformations[n].x += 5;
    } else if (input == 's') {
      correct(points, 0, 5, n);
      transformations[n].y += 5;
    } else if (input == 'w') {
      correct(points, 0, -5, n);
      transformations[n].y -= 5;
    } else if (input == 'n') {
      scanf("%d", &n);
      n = (n<4)*n;
    }
  }
}

int dInit() {
  struct fb_var_screeninfo vinfo;
  struct fb_fix_screeninfo finfo;

  // Open the file for reading and writing
  fbfd = open("/dev/fb0", O_RDWR);
  if (!fbfd) {
    printf("Error: cannot open framebuffer device.\n");
  }
  printf("The framebuffer device was opened successfully.\n");

  // Get fixed screen information
  if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
    printf("Error reading fixed information.\n");
  }

  // Get variable screen information
  if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
    printf("Error reading variable information.\n");
  }
  printf("%dx%d, %d bpp\n", vinfo.xres, vinfo.yres, 
         vinfo.bits_per_pixel );

  // map framebuffer to user memory 
  screensize = finfo.smem_len;

  fbp = (short*)mmap(0, 
                    screensize, 
                    PROT_READ | PROT_WRITE, 
                    MAP_SHARED, 
                    fbfd, 0);

  if ((int)fbp == -1) {
    printf("Failed to mmap.\n");
  }

  buffer = (short*)malloc(screensize);
  isDInit = 1;

  // hide cursor
  char *kbfds = "/dev/tty";
  int kbfd = open(kbfds, O_WRONLY);
  if (kbfd >= 0) {
      ioctl(kbfd, KDSETMODE, KD_GRAPHICS);
  }
  else {
      printf("Could not open %s.\n", kbfds);
  }
  return 0;
}

int end() {
  // cleanup
  munmap(fbp, screensize);
  close(fbfd);
}

int display()
{
  if (!isDInit) {
    dInit();
  }
  // loop(fbp, buffer, screensize);
  struct point2d points[4] = {
    (struct point2d){WIDTH-100, (HEIGHT)-100}, 
    (struct point2d){WIDTH-1100, (HEIGHT)-100}, 
    (struct point2d){WIDTH-1100, (HEIGHT)-600}, 
    (struct point2d){WIDTH-100, (HEIGHT)-600}};
  // short color = getColor(0, 255, 255);
  memset(buffer, getColor(255, 0,255), screensize);
  // fillConvexPolygon(base_color, 4, points);
  render();
  // struct mousePos pos = getMousePos();
  // printf("x=%d, y=%d, left=%d, middle=%d, right=%d\n", pos.x, pos.y, pos.left, pos.middle, pos.right);
  return 0;
}

int on() {
  return system("echo 'on 0.0.0.0' | cec-client -s -d 1");
}

int off() {
  return system("echo 'standby 0.0.0.0' | cec-client -s -d 1");
}

void setColor(int r, int g, int b) {
  base_color = getColor(r, g, b);
}

unsigned short blendColors(unsigned short* colors, float* weights, int numColors) {
  float r = 0;
  float g = 0;
  float b = 0;

  for (int i = 0; i<numColors; i++) {
    r += (colors[i] >> 11) * weights[i];
    g += ((colors[i] & (63 << 5)) >> 5) * weights[i];
    b += (colors[i] & 31) * weights[i];
  }
  return ((short)r << 11) + ((short)g << 5) + (short)b;
}

void transformProjection(float xRatio, float yRatio, int shift) {
  short* tbuffer = (short*)malloc(screensize);
  int offset;
  float yLayer = 0;
  float weights[2];
  float compression;
  float aspectRatio = (((float) WIDTH)/HEIGHT);
  float margin = (1 - xRatio)/2.0;
  for (int i = 0; i < HEIGHT; i++) {
    offset = (int)(margin * (i) * aspectRatio);
    // printf("%d\t", offset);
    compression = 1 / (1 - ((1 - xRatio) * (i / (float) HEIGHT)));
    // printf("%f\n", compression);
    for (int j = 0; j < (WIDTH - offset*2); j++) {
      float pos = j * compression;
      weights[1] = pos - ((int)pos);
      weights[0] = ((int)pos) + 1 - pos;
      tbuffer[WIDTH*i + offset + j] = blendColors(&buffer[WIDTH*i + (int)pos], weights, 2);
    }
  }

  memset(buffer, getColor(0, 0, 0), screensize);

  for (int i = 0; i < HEIGHT; i++) {
    if (i+shift>HEIGHT) {
      break;
    }
    float ystep = 1 / (1 - ((1 - yRatio) * i/((float) HEIGHT)));
    for (int j = 0; j < WIDTH; j++) {
      weights[1] = yLayer - ((int)yLayer);
      weights[0] = ((int)yLayer) + 1 - yLayer;
      buffer[WIDTH*(i+shift) + j] = blendColors((short[2]){tbuffer[WIDTH*(int)yLayer + j], tbuffer[WIDTH*(int)(yLayer+1) + j]}, weights, 2);
    }
    yLayer += ystep;
    if (yLayer >= HEIGHT) {
      break;
    }
  }
  memcpy(fbp, buffer, WIDTH*HEIGHT*2);
  free(tbuffer);
}

void displayBitMap(char* path, int rotate, int xmirror, int ymirror, float xScale, float yScale, int offset) {
  if (!isDInit) {
    dInit();
  }
  memset(buffer, getColor(0, 0, 0), screensize);
  char* fbuffer = (char*)malloc(0x36);
  FILE* infile = fopen(path, "r");
  if(infile == NULL) {
    printf("File not found: %s", path);
    return;
  }
  fseek(infile, 0L, SEEK_SET);
  fread(fbuffer, sizeof(char), 0x36, infile);
  short fileType = (((short)fbuffer[0]) << 8) | (short)fbuffer[1];
  if(fileType != 0x424d) {
    printf("File type incorrect %d", fileType);
    return;
  }

  int bmWidth = (((short)fbuffer[0x13]) << 8) | (short)fbuffer[0x12];
  int bmHeight = (((short)fbuffer[0x17]) << 8) | (short)fbuffer[0x16];
  if (rotate) {
    int swp = bmWidth;
    bmWidth = bmHeight;
    bmHeight = swp;
  }
  short bmColor;

  fbuffer = (char*)malloc(bmHeight*bmWidth*3);
  fread(fbuffer, sizeof(char), bmHeight*bmWidth*3, infile);

  int iWidth;
  int iHeight;

  if (HEIGHT > bmHeight) {
    iHeight = bmHeight;
  } else {
    iHeight = HEIGHT;
  }

  if (WIDTH > bmWidth) {
    iWidth = bmWidth;
  } else {
    iWidth = WIDTH;
  }

  int fIndex;
  int gIndex; 
  for (int i = 0; i<iHeight; i++) {
    for (int j = 0; j<iWidth; j++) {
      if (rotate){
        fIndex = (bmHeight - i + (bmWidth-j)*bmHeight)*3;
      } else {
        fIndex = (i*bmWidth + j)*3;
      }
      if (ymirror) {
        gIndex = (HEIGHT - i) * WIDTH;
      } else {
        gIndex = i*WIDTH;
      }
      if (xmirror) {
        gIndex += WIDTH - j;
      } else {
        gIndex += j;
      }
      bmColor = getColor((int)fbuffer[fIndex+2], (int)fbuffer[fIndex + 1], (int)fbuffer[fIndex]);
      buffer[gIndex] = bmColor;
    }
  }
  fclose(infile);
  //render();
  transformProjection(xScale, yScale, offset);
}


int main(int argc, char* argv[]) {
  dInit();
  displayBitMap("/home/pi/Projects/CPP/graphics_test/images/Our_Good_Night_Story_-_Book/2.bmp", 1, 1, 1, 0.8, 0.51, 300);
  end();
  return 0;
}
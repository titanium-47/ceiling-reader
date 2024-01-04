#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

struct mousePos {
    signed char x;
    signed char y;
    int left;
    int right;
    int middle; 
};

const char *pDevice = "/dev/input/mice";
    

struct mousePos getMousePos(){
    int fd, bytes;
    unsigned char data[3];

    fd = open(pDevice, O_RDWR);
    if(fd == -1)
    {
        printf("ERROR Opening %s\n", pDevice);
    }

    struct mousePos pos = {0,0,0,0,0};
    // Read Mouse     
    bytes = read(fd, data, sizeof(data));
    if(bytes > 0)
    {
        pos.left = data[0] & 0x1;
        pos.right = (data[0] & 0x2)>>1;
        pos.middle = data[0] & 0x4>>2;

        pos.x = data[1];
        pos.y = data[2];
        printf("x=%d, y=%d, left=%d, middle=%d, right=%d\n", pos.x, pos.y, pos.left, pos.middle, pos.right);
    }
    close(fd);  
    return pos; 
}

int getClick() {
    int fd, bytes;
    unsigned char data[3];
    
    fd = open(pDevice, O_RDWR);
    if(fd == -1)
    {
        printf("ERROR Opening %s\n", pDevice);
    }

    bytes = read(fd, data, sizeof(data));
    write(fd, (char*){"\0", "\0", "\0"}, 3);
    close(fd);
    return data[0];
}

// int main() {
//     printf("%d", getClick());
//     // getMousePos();
// }
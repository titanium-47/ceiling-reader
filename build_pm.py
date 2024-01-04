from cffi import FFI
import os
from dotenv import load_dotenv

load_dotenv()
PROJECT_PATH = os.getenv("PROJECT_PATH")

CDEF = '''\
int dInit();
int end();
void displayBitMap(char* path, int rotate, int xmirror, int ymirror, float xScale, float yScale, int offset);
int on();
int off();
int getClick();
void setColor(int r, int g, int b);
'''

ffibuilder = FFI()
ffibuilder.cdef(CDEF)
ffibuilder.set_source(
    '_projection_mapping',
    f'#include \"{PROJECT_PATH}/c_dependencies/display.c\"'
)
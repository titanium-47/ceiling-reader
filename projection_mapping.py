from _projection_mapping import ffi, lib

def displayImage(path):
    lib.displayBitMap(ffi.new("char[]", bytes(path, "utf-8")), 1, 1, 1, 0.8, 0.1, 350)

def getClick() -> (int, int, int, int):
    raw = lib.getClick()
    return(raw&1, raw&2>>1, raw&4>>2, raw)

def projectorOn():
    lib.on()

def projectorOff():
    lib.off()

if __name__ == "__main__":
    # displayImage("test")
    # print(getClick())
    lib.off()
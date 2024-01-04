from flask import Flask, jsonify, render_template, request, make_response
from werkzeug.utils import secure_filename
import os
from dotenv import load_dotenv
import ghostscript
import PyPDF2
from projection_mapping import displayImage, projectorOn, projectorOff
import json

load_dotenv()
PROJECT_PATH = os.getenv("PROJECT_PATH")

app = Flask(__name__)
app.config['MAX_CONTENT_PATH'] = 5000000
app.config['UPLOAD_FOLDER'] = f"{PROJECT_PATH}/images"

@app.route('/')   
def main():   
    return render_template("index.html") 

@app.route('/upload', methods=['POST'])
def upload():
    if request.method == 'POST':
        f = request.files['file']
        fname = secure_filename(f.filename)
        fpath = f"{PROJECT_PATH}/pdfs/{fname}"
        f.save(fpath)
        if (not os.path.exists(f"{PROJECT_PATH}/info.json")):
            with open(f"{PROJECT_PATH}/info.json", "w") as write_file:
                json.dump({"Books": {}}, write_file)
        with open(f"{PROJECT_PATH}/info.json", "r+") as file:
            data = json.load(file)
            file.seek(0)
            data["Books"][fname[:len(fname)-4]] = {
                    "TotalPageCount": len(PyPDF2.PdfReader(fpath).pages),
                    "ReaderOnPage": 1,
                    "PagesLoaded": [],
                    "PDFPath": fpath,
                    "ImagePath": f'{PROJECT_PATH}/images/{fname[:len(fname)-4]}'
                }
            data["MostRecent"] = fname[:len(fname)-4]
            json.dump(data, file)
        saveTitle(fpath, fname[:len(fname)-4])
        loadPage(fname[:len(fname)-4], 1)
        loadPage(fname[:len(fname)-4], 2)        
        return fname[:len(fname)-4]  
    
@app.route("/nextPage", methods=["POST"])
def nextPage():
    if request.method == 'POST':
        data = {}
        with open(f"{PROJECT_PATH}/info.json", "r") as file:
            data = json.load(file)
        
        data["Books"][data["MostRecent"]]["ReaderOnPage"]+=1
        book = data["Books"][data["MostRecent"]]
        displayImage(f'{book["ImagePath"]}/{book["ReaderOnPage"]}.bmp')
        loadPage(data["MostRecent"], book["ReaderOnPage"]+1)
        deletePage(data["MostRecent"], book["ReaderOnPage"]-2)

        with open(f"{PROJECT_PATH}/info.json", "w") as file:
            json.dump(data, file)
        return "success"

@app.route("/prevPage", methods=["POST"])
def prevPage():
    if request.method == 'POST':
        data = {}
        with open(f"{PROJECT_PATH}/info.json", "r") as file:
            data = json.load(file)
        
        data["Books"][data["MostRecent"]]["ReaderOnPage"]-=1
        book = data["Books"][data["MostRecent"]]
        displayImage(f'{book["ImagePath"]}/{book["ReaderOnPage"]}.bmp')
        loadPage(data["MostRecent"], book["ReaderOnPage"]-1)
        deletePage(data["MostRecent"], book["ReaderOnPage"]+2)

        with open(f"{PROJECT_PATH}/info.json", "w") as file:
            json.dump(data, file)
        return "success"

@app.route("/openPage", methods=["POST"])
def openPage():
    if request.method == 'POST':
        rBook = request.get_json()["Book"]
        data = {}
        with open(f"{PROJECT_PATH}/info.json", "r") as file:
            data = json.load(file)
            if rBook is None:
                book = data["Books"][data["MostRecent"]]
            else:
                book = data["Books"][rBook]
                data["MostRecent"] = rBook
            displayImage(f'{book["ImagePath"]}/{book["ReaderOnPage"]}.bmp')
        with open(f"{PROJECT_PATH}/info.json", "w") as file:
            json.dump(data, file)
        return "success"

@app.route("/getBooks", methods=["POST"])    
def getBooks():
    books = []
    if request.method == 'POST':
        with open(f"{PROJECT_PATH}/info.json", "r") as file:
            data = json.load(file)
            books = list(data["Books"].keys())
        return jsonify(books)
    
@app.route("/projectorOn", methods=['POST'])
def on():
    projectorOn()
    return make_response({'status': 200})

@app.route("/projectorOff", methods=['POST'])
def off():
    projectorOff()
    return make_response({'status': 200})

def saveTitle(pdfPath, imageName):
    args = ["pdf2jpeg", # actual value doesn't matter
                "-dNOPAUSE",
                "-sDEVICE=jpeg",
                "-r128",
                f"-dFirstPage={1}",
                f"-dLastPage={1}",
                f"-sOutputFile={PROJECT_PATH}/static/{imageName}.jpeg",
                pdfPath]
    ghostscript.Ghostscript(*args)

def loadPage(book_name, page):
    with open(f"{PROJECT_PATH}/info.json", "r+") as file:
        data = json.load(file)
        file.seek(0)
        book = data["Books"][book_name]
        if (page > book["TotalPageCount"] or page<1):
            return
        if not os.path.exists(book["ImagePath"]):
            os.mkdir(book["ImagePath"])

        iname = f'{book["ImagePath"]}/{page}.bmp'
        if (not os.path.isfile(iname)):
            f = open(iname, 'x')
            f.close()

        args = ["pdf2bmp", # actual value doesn't matter
                "-dNOPAUSE",
                "-sDEVICE=bmp16m",
                "-r128",
                f"-dFirstPage={page}",
                f"-dLastPage={page}",
                "-sOutputFile=" + iname,
                book["PDFPath"]]
        ghostscript.Ghostscript(*args)
        data["Books"][book_name]["PagesLoaded"].append(page)
        json.dump(data, file)
        
def deletePage(book_name, page):
     with open(f"{PROJECT_PATH}/info.json", "r+") as file:
        data = json.load(file)
        file.seek(0)
        book = data["Books"][book_name]
        if page > book["TotalPageCount"] or page<1:
            return
        iname = f'{book["ImagePath"]}/{page}.bmp'
        os.remove(iname)
        if page in data["Books"][book_name]["PagesLoaded"]:
            data["Books"][book_name]["PagesLoaded"].remove(page)
        json.dump(data, file)

if __name__ == "__main__":
    app.run('192.168.1.21', debug=True, port=8000,)

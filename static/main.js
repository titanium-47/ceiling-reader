fetch('/getBooks', {
    method: 'POST',
})
.then(response => response.json())
.then(books => {
    const booksContainer = document.getElementById('root');
    books.forEach(book => {
        const bookBox = document.createElement('div');
        bookBox.classList.add('book-box')
        const bookImg = document.createElement('img');
        bookImg.classList.add('book-img');
        bookImg.src = "/static/" + book + ".jpeg";
        bookImg.alt = book;
        bookBox.id = book;
        bookBox.appendChild(bookImg);
        booksContainer.prepend(bookBox);
        addBookListener(book);
    });
})
.catch(error => console.error('Error:', error));

document.getElementById('onButton').addEventListener('click', function() {
    fetch('/projectorOn', {
        method: 'POST',
    })
    .then(response => response.json())
    .then(data => console.log(data))
    .catch(error => console.error('Error:', error));
});

document.getElementById('offButton').addEventListener('click', function() {
    fetch('/projectorOff', {
        method: 'POST',
    })
    .then(response => response.json())
    .then(data => console.log(data))
    .catch(error => console.error('Error:', error));
});

document.getElementById('uploadBox').addEventListener('click', function() {
    document.getElementById('fileInput').click();
});

function addBookListener(book) {
    document.getElementById(book).addEventListener('click', function() {
        fetch('/openPage', {
            method: 'POST', 
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({"Book": book})
        })
    });
}
document.getElementById('fileInput').addEventListener('change', function() {
    let file = this.files[0];
    uploadFile(file);
});

function uploadFile(file) {
    let url = '/upload';
    let formData = new FormData();

    formData.append('file', file);

    fetch(url, {
        method: 'POST',
        body: formData
    })
    .then(response => response.text())
    .then(book => {
        console.log(book);
        const booksContainer = document.getElementById('root');
        const bookBox = document.createElement('div');
        bookBox.classList.add('book-box')
        const bookImg = document.createElement('img');
        bookImg.classList.add('book-img');
        bookImg.src = "/static/" + book + ".jpeg";
        bookImg.alt = book;
        bookBox.id = book;
        bookBox.appendChild(bookImg);
        booksContainer.prepend(bookBox);
        addBookListener(book);
    }); 
}

document.getElementById('forwardButton').addEventListener('click', function() {
    fetch('/nextPage', {
        method: 'POST',
    })
});

document.getElementById('backButton').addEventListener('click', function() {
    fetch('/prevPage', {
        method: 'POST',
    })
});

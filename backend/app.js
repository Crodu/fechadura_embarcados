var express = require('express');
const cors = require('cors');
var app = express();
var bodyParser = require('body-parser');
var sqlite = require('sqlite3');
var env = require('dotenv').load();
var port = process.env.PORT || 8080;

// models
var models = require("./models");

// routes
var books = require('./routes/books');
var users = require('./routes/users');
var admin = require('./routes/admin');
var management = require('./routes/management');
var register = require('./routes/register');

//Sync Database
models.sequelize.sync().then(function() {
    console.log('connected to database')
}).catch(function(err) {
    console.log(err)
});

app.use(cors())
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({
    extended: true
}));

// register routes
app.use('/books', books);
app.use('/users', users);
app.use('/admin', admin);
app.use('/command', management);
app.use('/register', register);

// index path
app.get('/', function(req, res){
    console.log('app listening on port: '+port);
    res.send('tes express nodejs sqlite')
});

app.listen(port, function(){
    console.log('app listening on port: '+port);
});

module.exports = app;
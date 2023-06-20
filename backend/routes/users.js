var express = require('express');
var User = require('../models').User;
var router = express.Router();

router.post('/', function(req, res){
    //console.log('Adding user');
    User.create({
        username: req.body.username,
        password: req.body.password,
        rfid_tag: req.body.rfid_tag
    }).then(user => {
        res.status(200).json(user);
    });
});

router.get('/', function(req, res){
    //console.log('Getting all users');
    User.findAll().then(users => {
        res.status(200).json(users);
    });
});

module.exports = router;
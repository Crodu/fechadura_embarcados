var express = require('express');
var User = require('../models').User;
var router = express.Router();

router.get('/', function(req, res){
    //console.log('Getting all users');
    User.findAll().then(users => {
        res.status(200).json(users);
    });
});

router.post('/', function(req, res){
    //console.log('Adding user');
    User.create({
        name: req.body.name,
        password: req.body.password,
        rfid_tag: req.body.rfid_tag
    }).then(user => {
        res.status(200).json(user);
    });
});


module.exports = router;
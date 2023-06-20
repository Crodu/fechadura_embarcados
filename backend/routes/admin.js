var express = require('express');
var Admin = require('../models').Admin;
var router = express.Router();

// middleware

router.post('/auth', function(req, res){
    // get username and password from the request
    var username = req.body.username;
    var password = req.body.password;

    // find the user in the database
    Admin.findOne({ where: { username: username } }).then(admin => {
        // check if user exists
        if (!admin) {
            res.status(404).json('User not found: '+username+'');
        } else {
            // check if password matches
            if (admin.password === password) {
                res.status(200).json(admin);
            } else {
                res.status(401).json('Incorrect password');
            }
        }
    });
});

module.exports = router;
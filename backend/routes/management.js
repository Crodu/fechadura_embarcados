var express = require('express');
var Status = require('../models').Status;
var router = express.Router();

router.get('/', function(req, res){
    //console.log('Getting current command');
    const comando = {
        cadastro: false,
        sensor: 'biometria',
    }
    Status.findOne().then(user => {
        res.status(200).json(user);
    });
    // res.status(200).json(comando);
});

module.exports = router;
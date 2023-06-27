var express = require('express');
var Status = require('../models').Status;
var router = express.Router();

router.get('/', function(req, res){
    //console.log('Getting current command');
    const comando = {
        cadastro: false,
        sensor: 'biometria',
    }
    Status.findOne().then(status => {
        res.status(200).json(status);
    });
    // res.status(200).json(comando);
});

router.post('/', function(req, res){
    //console.log('updating command');
    Status.update({
        isRegistering: req.body.isRegistering,
        currentSensor: req.body.currentSensor,
        userId: req.body.userId,
    }, {where: {id: 1}}).then(status => {
        res.status(200).json(status);
    });
});

module.exports = router;
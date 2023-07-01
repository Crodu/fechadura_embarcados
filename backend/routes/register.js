var express = require('express');
var User = require('../models').User;
var Status = require('../models').Status;
var router = express.Router();

const resetStatus = () => {
    Status.update({
        isRegistering: false,
        userId: null,
    }, {where: {id: 1}});
}

router.get('/', function(req, res){
    //console.log('Getting all users');
    let {
        userId,
        sensor,
        data = '',
    } = req.query
    
    if (sensor === 'biometria'){
        data = data.split('239,1,255,255,255,255,2,0,130,');
        User.update({
            posicao: userId,
            pacote1: data[1] || '',
            pacote2: data[2] || '',
            pacote3: data[3] || '',
            pacote4: data[4] || '',
        },{where: {id: userId}}).then(user => {
            res.status(200).json(user);
            resetStatus();
        });
    }else if (sensor === 'rfid'){
        User.update({
            rfid_tag: data,
        },{where: {id: userId}}).then(user => {
            res.status(200).json(req.query);
            resetStatus();
        });
    }

    else res.status(200).json({status: 'sucesso', userId, sensor, data});

    
});

module.exports = router;
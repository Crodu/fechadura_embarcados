var express = require('express');
var User = require('../models').User;
var router = express.Router();

router.get('/', function(req, res){
    //console.log('Getting all users');
    let {
        userId,
        sensor,
        data,
    } = req.query
    
    if (sensor === 'biometria'){
        data = data.split('239,1,255,255,255,255,2,0,130,');
        User.update({
            posicao: userId,
            pacote1: data[1],
            pacote2: data[2],
            pacote3: data[3],
            pacote4: data[4],
        },{where: {id: userId}}).then(user => {
            res.status(200).json(user);
        });
    }

    else res.status(200).json({status: 'sucesso', userId, sensor, data});

    
});

module.exports = router;
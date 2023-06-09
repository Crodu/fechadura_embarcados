var express = require('express');
var User = require('../models').User;
var AuthLog = require('../models').AuthLog;
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

router.delete('/', function(req, res){
    //console.log('Adding user');
    User.update({
        status: 0
    },{where: {id: req.body.userId}}).then(user => {
        res.status(200).json(user);
    });
});


router.get('/', function(req, res){
    //console.log('Getting all users');

    //join users with authlogs
    User.findAll({
    where: {status: 1},
    include: [{
        model: AuthLog,
        order: [['createdAt', 'DESC']],
        limit: 1,
    }]}).then(users => {
        res.status(200).json(users);
    });
    return

    User.findAll().then(users => {
        res.status(200).json(users);
    });
});

router.get('/auth', function(req, res){
    let {
        password,
        rfid,
        posicao,
    } = req.query

    if (password) {
        User.findOne({where: {password: password}}).then(user => {
            if(user){
                AuthLog.create({
                    userId: user.id,
                    method: 'password',
                    success: true,
                })
                res.status(200).json({message: 'Success'});
            }else{
                res.status(404).json({error: 'User not found'});
            }
            
        });
    }else if (rfid) {
        User.findOne({where: {rfid_tag: rfid}}).then(user => {
            if(user){
                AuthLog.create({
                    userId: user.id,
                    method: 'rfid',
                    success: true,
                })
                res.status(200).json({message: 'Success'});
            }else{
                res.status(404).json({error: 'User not found'});
            }
            
        });
    }else if (posicao) {
        User.findOne({where: {posicao: posicao}}).then(user => {
            if(user){
                AuthLog.create({
                    userId: user.id,
                    method: 'biometrics',
                    success: true,
                })
                res.status(200).json({message: 'Success'});
            }else{
                res.status(404).json({error: 'User not found'});
            }
            
        });
    }
});

router.get('/lastpos', function(req, res){
    //Get user with hihghest position number
    User.findOne({order: [['posicao', 'DESC']]}).then(user => {
        res.status(200).json({id: user.id, posicao: user.posicao});
    });
});

router.get('/template', function(req, res){
    //Get all packages from user with given position
    let {
        pos,
    } = req.query
    User.findOne({where: {posicao: pos}}).then(user => {
        res.status(200).json(user);
    });
});


module.exports = router;
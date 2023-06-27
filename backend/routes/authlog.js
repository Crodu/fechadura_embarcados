var express = require('express');
var AuthLog = require('../models').AuthLog;
var router = express.Router();

router.get('/log/', function(req, res){
    //console.log('Adding AuthLog');
    let {
        userId,
        method,
        success,
    } = req.query

    AuthLog.create({
        userId,
        method,
        success,
    }).then(log => {
        res.status(200).json(log);
    });
});

router.get('/', function(req, res){
    //console.log('Getting all users');
    AuthLog.findAll({
        attributes: ['userId', 'method', 'success', [AuthLog.sequelize.literal(`DATE("createdAt")`), 'date'], [AuthLog.sequelize.literal(`COUNT(*)`), 'count']],
        group: ['date'],
    }).then(logs => {
        res.status(200).json(logs);
    });
});



module.exports = router;
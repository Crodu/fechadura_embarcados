module.exports = function(sequelize, Sequalize) {
    var StatusSchema = sequelize.define("Status", {
        isRegistering: Sequalize.BOOLEAN,
        currentSensor: Sequalize.STRING,
        userId: Sequalize.INTEGER,
    },{
        timestamps: false
    });
    return StatusSchema;
}
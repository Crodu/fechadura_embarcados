module.exports = function(sequelize, Sequalize) {
    var UserSchema = sequelize.define("User", {
        username: Sequalize.STRING,
        password: Sequalize.STRING,
        rfid_tag: Sequalize.STRING
    },{
        timestamps: false
    });
    return UserSchema;
}
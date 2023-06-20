module.exports = function(sequelize, Sequalize) {
    var AdminSchema = sequelize.define("Admin", {
        username: Sequalize.STRING,
        password: Sequalize.STRING,
        email: Sequalize.STRING
    },{
        timestamps: false
    });
    return AdminSchema;
}
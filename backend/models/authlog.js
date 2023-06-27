module.exports = function(sequelize, Sequalize) {
    let AuthLogSchema = sequelize.define("AuthLog", {
        userId: Sequalize.INTEGER,
        method: Sequalize.STRING,
        success: Sequalize.BOOLEAN,
    },{
        timestamps: true
    });

    AuthLogSchema.associate = function(models){
        AuthLogSchema.belongsTo(models.User, {foreignKey: 'userId'});
    }

    return AuthLogSchema;
}
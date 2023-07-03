module.exports = function(sequelize, Sequalize) {
    var UserSchema = sequelize.define("User", {
        username: Sequalize.STRING,
        password: Sequalize.STRING,
        rfid_tag: Sequalize.STRING,
        posicao: Sequalize.INTEGER,
        pacote1: Sequalize.STRING,
        pacote2: Sequalize.STRING,
        pacote3: Sequalize.STRING,
        pacote4: Sequalize.STRING,
        status: Sequalize.BOOLEAN,
    },{
        timestamps: false
    });

    UserSchema.associate = function(models){
        UserSchema.hasMany(models.AuthLog, {foreignKey: 'userId'});
    }

    return UserSchema;
}
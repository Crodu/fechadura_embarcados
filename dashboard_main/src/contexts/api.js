export const BACKEND_URL = 'http://localhost:8080';

export const setCommand = (command, auth) => {
    return fetch(BACKEND_URL + '/command', {
        method: 'POST',
        headers: {
        'Content-Type': 'application/json'
        },
        body: JSON.stringify(command)
    }).then((response) => {
        auth.updateStatus();
    });
}

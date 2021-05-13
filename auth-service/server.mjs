import mqtt from 'mqtt'
import validCards from './config/auth.json'

const client = mqtt.connect('mqtt://broker', {
    username: 'esp8266',
    password: 'irob-is-cool',
})

const sleep = (ms = 500) => new Promise((res) => {
    setTimeout(res, ms)
})

/**
 * @param {string} cardUid
 * @returns {Promise<void>}
 */
const handleAccessRequest = async (cardUid) => {
    if (!(cardUid in validCards)) {
        console.log(`Did not recognise "${cardUid}"`)
        return
    }

    const info = validCards[cardUid]
    console.log(`Received card with name "${info.name}"`)
    if (info.access !== true) {
        console.log('  Card does not have access...')
        return
    }
    console.log('  Card has access')
    client.publish('door/state', 'open')
    await sleep(5000)
    client.publish('door/state', 'close')
}

const handlers = {
    'door/access': handleAccessRequest
}

client.on('connect', () => {
    console.log('connected!')
    client.subscribe('door/access', console.log)
})

client.on('message', (topic, message) => {
    const topicHandler = handlers[topic]

    if (typeof topicHandler === 'function') {
        topicHandler(message.toString())
    }
})

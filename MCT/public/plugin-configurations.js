function Configs(){
    return {
            battery: {
                namespace: 'bat.taxonomy',
                key: 'spacecraft',
                type: 'bat.telemetry',
                typeName: 'Battery Telemetry Point',
                typeDescription: 'Telemetry point on the Gomspace battery',
                typeCssClass: 'icon-telemetry',
                jsonFile: '/battery.json'
            },

            dictionary:{
                namespace: 'sat.taxonomy',
                key: 'spacecraft',
                type: 'sat.telemetry',
                typeName: 'Satellite Telemetry Point',
                typeDescription: 'Telemetry point on the satellite to send data',
                typeCssClass: 'icon-telemetry',
                jsonFile: '/dictionary.json'
            }

    }
}
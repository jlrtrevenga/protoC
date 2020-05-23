# Boiler Control Prototype C

ESP32 Heater Controller

Actual Status:

- Main structure design: 
    - Independent tasks, start/stop on demand. 
    - intercommunication via events (not yet finished)
    - Core common structure to implement functionalities on top of core services.

Tasks/functionalities:
    - Core services:
        - i2c comms task.
            - BMP280 reading.
        - gpio interrupt task handle.
            - OUTPUT relay control.
        - Wifi + ntp task (correct recovery on comm failure)
        - MQTT comm task.
    - Process functionalities:
        - Weekly temperature programm + Output relay control.


Pending:
    - I/O structure reorganization.
    - complete modular task structure.
    - GSM communications.
    - ESP Now / ESP mesh comm to secondary modules.
    - JTAG enable (prototipe)
    - check MQTT reconnection (some errors detected)
    - JSON structure (comms to Heater control app)
    - Security:
        - Alternative 1: 
            - MQTT: certificates use.
        - Alternative 2:
            - Integration to Azure / AWS Services.
            - Include provisioning service + certificates.



## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites

What things you need to install the software and how to install them

```
Give examples
```

### Installing

A step by step series of examples that tell you how to get a development env running

Say what the step will be

```
Give the example
```

And repeat

```
until finished
```

End with an example of getting some data out of the system or using it for a little demo

## Running the tests

Explain how to run the automated tests for this system

### Break down into end to end tests

Explain what these tests test and why

```
Give an example
```

### And coding style tests

Explain what these tests test and why

```
Give an example
```

## Deployment

Add additional notes about how to deploy this on a live system

## Built With

* [Dropwizard](http://www.dropwizard.io/1.0.2/docs/) - The web framework used
* [Maven](https://maven.apache.org/) - Dependency Management
* [ROME](https://rometools.github.io/rome/) - Used to generate RSS Feeds

## Contributing

Please read [CONTRIBUTING.md](https://gist.github.com/PurpleBooth/b24679402957c63ec426) for details on our code of conduct, and the process for submitting pull requests to us.

## Versioning

We use [SemVer](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](https://github.com/your/project/tags). 

## Authors

* **Billie Thompson** - *Initial work* - [PurpleBooth](https://github.com/PurpleBooth)

See also the list of [contributors](https://github.com/your/project/contributors) who participated in this project.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* Hat tip to anyone whose code was used
* Inspiration
* etc

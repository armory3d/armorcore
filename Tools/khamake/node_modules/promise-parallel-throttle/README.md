# Promise-parallel-throttle
[![Build Status](https://travis-ci.org/DJWassink/Promise-parallel-throttle.svg?branch=master)](https://travis-ci.org/DJWassink/Promise-parallel-throttle)

Run a array of Promises in parallel. Kinda like Promise.all(), but throttled!

## Install 

### NPM
```bash
npm i promise-parallel-throttle -S
```

### Yarn
```bash
yarn add promise-parallel-throttle
```

## Usage

```js
import Throttle from 'promise-parallel-throttle';

//Function which should return a Promise
const doReq = async (firstName, lastName) => {
    //Do something async.
    return firstName + " " + lastName;
}

const users = [
    {firstName: "Irene", lastName: "Pullman"},
    {firstName: "Sean",  lastName: "Parr"}
];

//Queue with functions to be run
const queue = users.map(user => () => doReq(user.firstName, user.lastName));

//Default Throttle runs with 5 promises parallel.
const formattedNames = await Throttle.all(queue);

console.log(formattedNames); //['Irene Pullman', 'Sean Parr']
```

## API
### Throttle.all
`Throttle.all(tasks, options)`

Throttle.all is made to behave exactly like Promise.all but instead of all the tasks running in parallel it runs a maxium amount of tasks in parallel.
Only the tasks parameter is required while the [options](#options-object) parameter is optional.

### Throttle.sync
`Throttle.sync(tasks, options)`

Throttle.sync runs all the tasks synchronously. 
Once again the tasks array is required, the [options](#options-object) are optional. 
Be aware that this method is simply a wrapper to pass `maxInProgress` with 1. So overwriting this option in the options object would run the tasks again in parallel.

### Throttle.raw
`Throttle.raw(tasks, options)`
 
 The raw method instead of returning the tasks their results, will return a [result](#result-object--progress-callback) object. 
 Useful if you wan't more statistics about the execution of your tasks. Once again the tasks are required while the [options](#options-object) are optional.

#### Option's Object
|Parameter|Type|Default|Definition|
|:---|:---|:---|:---|
|maxInProgress |Integer|5| max amount of parallel threads|
|failFast |Boolean|true (false for the [raw](#throttleraw) method)| reject after a single error, or keep running|
|progressCallback |Function|Optional| callback with progress reports|
|nextCheck |Function|Optional| function which should return a promise, if the promise resolved true the next task is spawn, errors will propagate and should be handled in the calling code|

#### Result object / Progress callback
The `progressCallback` and the `Raw` will return a `Result` object with the following properties:

|Property|Type|Start value|Definition|
|:---|:---|:---|:---|
|amountDone|Integer|0|amount of tasks which are finished|
|amountStarted|Integer|0|amount of tasks which started|
|amountResolved|Integer|0|amount of tasks which successfully resolved|
|amountRejected|Integer|0|amount of tasks which returned in an error and are aborted|
|amountNextCheckFalsey|Integer|0|amount of tasks which got a falsey value in the [nextCheck](#nextcheck)|
|rejectedIndexes|Array|[]|all the indexes in the tasks array where the promise rejected|
|resolvedIndexes|Array|[]|all the indexes in the tasks array where the promise resolved|
|nextCheckFalseyIndexes|Array|[]|all the indexes in the tasks array where the [nextCheck](#nextcheck) returned a falsey value|
|taskResults|Array|[]|array containing the result of every task|

#### nextCheck
All the `Throttle` methods have a `nextCheck` method which will be used to verify if a next task is allowed to start. 

The default `nextCheck` is defined like this;
```js
const defaultNextTaskCheck = (status, tasks) => {
    return new Promise((resolve, reject) => {
        resolve(status.amountStarted < tasks.length);
    });
}
```

This function will get a status object as parameter which adheres to the [Result object](#result-object--progress-callback) and it also receives the list of tasks.
In the default `nextCheck` we simply check if the amount of started exceeds the amount to be done, if not we are free to start an other task.

This function can be useful to write your own scheduler based on, for example ram/cpu usage.
Lets say that your tasks use a lot of ram and you don't want to exceed a certain amount.
You could then write logic inside a `nextCheck` function which resolves after there is enough ram available to start the next task.

If a custom implementation decides to reject, the error is propagated and should be handled in the user it's code. If a custom implementation returns a falsey value the task will simply not execute and the next task will be scheduled.

## Example
Check out the example's directory, it's heavily documented so it should be easy to follow.

To run the example, at least Node 8.x.x is required, since it supports native async/await.

Simply run the example with npm:
```
npm run-script names
```

Or with Yarn:
```
yarn names
```

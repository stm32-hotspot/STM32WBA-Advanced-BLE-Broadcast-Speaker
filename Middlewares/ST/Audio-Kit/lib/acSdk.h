/**
  ******************************************************************************
  * @file    acSdk.h
  * @author  MCD Application Team
  * @brief   Audio Chain User SDK
  *******************************************************************************
  * @attention
  *
  * Copyright (c) 2019(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ********************************************************************************
  */


#ifndef _acSDK_
#define _acSDK_

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include <cmsis_compiler.h>
#include "audio_chain_sysIOs.h"
#include "audio_chain_sysIOs_conf.h"


#ifdef __cplusplus
extern "C"
{
#endif



/* Exported constants --------------------------------------------------------*/

#define AC_TRUE                        (1UL)        /*!< Flag: TRUE as uint32_t  */
#define AC_FALSE                       (0UL)        /*!< Flag: FALSE as uint32_t  */
#define AUDIO_CHAIN_DISABLE_TUNING     (1UL << 0UL) /*!< Flag: Disable Tuning  functionalities */
#define AUDIO_CHAIN_DISABLED           (1UL << 1UL) /*!< Flag: Disable the algorithm*/
#define AUDIO_CHAIN_DEFAULT_CYCLECOUNT (1UL << 2UL) /*!< Flag: Add the default cycle count manager callback to the algo, this callback will display the CPU load on the UART. to modify the default timeout of 1sec, see acAlgoSetCommonConfig*/


/* Exported types ------------------------------------------------------------*/

/*! @ingroup type*/
typedef void *acChunk; /*!< Chunk Handle */
/*!@ingroup type*/
typedef void *acAlgo; /*!< Algo Handle */
/*!@ingroup type*/
typedef void *acPipe; /*!< Pipe Handle */



/*!
@brief Audio playback state
@see acPipePlay
@ingroup type
*/
typedef enum acState_t
{
  AC_START,  /*!< Start the pipeline */
  AC_STOP,   /*!< Stop  the pipeline */
  AC_CLEANUP /*!< cleanup  the pipeline after an error */
} acState_t;

/*!
@brief Audio processing control callback
@ingroup type
This callback is used in combination with the common parameter AudioAlgo_setCommonConfig(hAlgo, "controlCb", cb_control);
It allows to expose an audio processing that will be called after the algo processing is done.
*/
typedef int32_t (*AC_PROCESSING_CB)(acAlgo algo);


/* Exported functions ------------------------------------------------------- */

/*!
@ingroup SdkApi
@brief Audio  acSDK initialization

First function to call before to create an Audio Graph

*/
int32_t acInitialize(void);

/*!
@ingroup SdkApi
@brief Audio acSDK termination

Last function to call after  Audio Graph destruction

*/
int32_t acTerminate(void);



/*!
@ingroup SdkApi
@brief return the current SDK version


*/
const char *acSdkVersion(void);

/*!
@ingroup SdkApi
@brief return the  board  name

*/
const char *acConfigBoard(void);




/**
*
@brief set the audio config from its name
@ingroup SdkApi
@param pConfigName standard audio config name such as :ID_02160216onb08FFFFFF,etc..

**/


int32_t acAudioConfigSet(const char *pConfigName);




/*!
@ingroup SdkApi
@brief Create an audio pipeline
@ingroup SdkApi

This function returns an audio pipe instance. This instance is mandatory to connect algo and chunk.

@see acPipeDelete
@param ppPipe pointer ref on a pipe instance handle
@return 0 if success or an error code
*/
int32_t acPipeCreate(acPipe *ppPipe);


/*!
@ingroup SdkApi
@brief Delete an audio pipeline

This function deletes an audio pipe instance. A pipe is automatically deleted when the pipe is stopped.

@param hPipe  the pipe instance
@return 0 if success or an error code
*/
int32_t acPipeDelete(acPipe hPipe);



/*!
@ingroup SdkApi
@brief Dump all algos available in the system.

 The system has a set of algorithms able to create instances
 The function allows to list all exposed algorithms on the console log.
 The number of algos exposed depends on the Firmware build.

@return 0 if success or an error code
*/
int32_t acAlgoListDump(void);



/*!
@ingroup SdkApi
@brief Dump all  algorithm parameters on the console log.

This functions is similar to acAlgoDump, but this function could be called
without a verifier handle, so you can call it before the system initialization
and check parameters exposed by the algo.

@see acAlgoDump
@return 0 if success or an error code
*/
int32_t acAlgoTemplateDump(char *pName);



/*!
@ingroup SdkApi
@brief   Reset all parameters to its default values

Each algo's  parameter has a preferred value, this value is automatically set at the algo creation.
This function allows to reset manually all parameters to its default value

@return 0 if success or an error code
*/
int32_t acResetDefault(acAlgo hAlgo);



/*!
@ingroup SdkApi
@brief   Create an algo instance

Audio chain provides a set of algorithms template, from this template, we can create one or several instance of this algo.
A algo template is identified by a name, This function allows to create an instance using its template name and create an instance having an Instance name.

@param hPipe Pipe handle
@param pAlgoName Template algorithm name
@param pInstanceName Instance algorithm name
@param ppAlgo The pointer receiving the instance handle
@param iAlgoFlags Some flags, default is 0
@return 0 if success or an error code
@ingroup SdkApi
*/
int32_t acAlgoCreate(acPipe hPipe, const char *pAlgoName, const char *pInstanceName, acAlgo *ppAlgo, uint32_t iAlgoFlags, const char *pDesc);

/*!
@ingroup SdkApi
@brief   Delete an algorithm element instance

It is not mandatory to delete manually an an instance, the instance will be automatically deleted when the pipeline will be stopped.
This function delete manually an algo instance.

@see acAlgoCreate
@param hAlgo Algorithms handle instance
@return 0 if success or an error code
*/
int32_t acAlgoDelete(acAlgo hAlgo);




/*!
@ingroup SdkApi
@brief   return the algo instance form its instance name

The pipe must be playing
@see acAlgoCreate
@param hPipe  the pipe instance
@param pInstanceName  the algo  instance naùe
@return the algo instance or NULL
*/
acAlgo acAlgoGetInstance(acPipe hPipe, const char *pInstanceName);

/*!
@ingroup SdkApi
@brief   Configure an algorithm parameter

An algorithm has a set of parameters, you can set these parameters using this function.
The setting works using the method key value. To set a parameter you need to know its name and its scope.
a parameter value can have several types, but it is always passed as a string.
It could be an integer, a float, or a flag, etc...
You can enumerate all parameters using the LiveTune a check the parameter name and the scope.
It is also possible to enumerate parameter descriptions on the console using the acAlgoDump(hAlgo) function.

You can use this function when the pipe is running or not according to the internal parameter properties.
When you set a parameter with a stopped pipe, the parameter is applied immediately.
When you set a parameter with a playing pipe, the parameter is not applied immediately.
You can set several parameters and then call the function acAlgoRequestUpdate().
All parameters modified previously will be applied at once.


@see acAlgoDelete
@param hAlgo algorithms handle instance
@param pKey   Key parameter name
@param pValue  Value string
@return 0 if success or an error code
*/
int32_t acAlgoSetConfig(acAlgo hAlgo, const char *pKey, const char *pValue);





/*!
@ingroup SdkApi
@brief   return  an algorithm parameter as string


@see acAlgoSetConfig
@param hAlgo algorithms handle instance
@param pKey   Key parameter name
@param pValue  Value string buffer
@param szValue  Value string buffer size
@return 0 if success or an error code
*/
int32_t acAlgoGetConfig(acAlgo hAlgo, const char *pKey, char *pValue, uint32_t szValue);


/*!
@ingroup SdkApi
@brief   Configure an algorithm parameter using a pointer

This function is like acAlgoSetConfig, but the value is an opaque pointer used by the algorithm.
Some algo  use a database packaged in a file. This parameters have an specific internal attribute (AUDIO_DESC_PARAM_TYPE_ADDRESS)

The SDK will not check the validity of the pointer. The validity will be done when the algo will start.
If you try to use this function with parameter without internal flag AUDIO_DESC_PARAM_TYPE_ADDRESS, the function will return an error.


@see acAlgoDelete
@param hAlgo Algorithms handle instance
@param pKey  Parameter name
@param pPointer  Opaque parameter
@return 0 if success or an error code
*/
int32_t acAlgoSetConfigPtr(acAlgo hAlgo, const char *pKey, const void *pPointer);



/*!
@ingroup SdkApi
@brief   Request immediate parameter update when the pipeline is started

Notice this function has a small latency, the parameter will be really applied during the next  processing loop.
This function will return an error if the pipe is not started

@param hAlgo Algorithms handle instance
@return 0 if success or an error code
*/
int32_t acAlgoRequestUpdate(acAlgo hAlgo);

/*!
@ingroup SdkApi
@brief   get a control algorithm parameter

An algorithm could have some control callback.
The control callback allows to the algo to expose a result after the processing.
Some algos exposing control callback: rms, spectrum.

When an algorithm exposes a callback, it can also expose control parameters.
The application can use these control parameters for its own usage (data presentation or routing ....)


The setting works using the method key value and an opaque pointer.
The variable value can have several types, and you must pay attention to the type accepted.
It could be a integer, a float, an array, or a flag.
You can enumerate all control parameters and the type accepted using dump functions.



@param hAlgo Algorithms handle instance
@param pKey  parameter name
@param pParameter opaque pointer
@return 0 if success or an error code
*/
int32_t acAlgoGetControl(acAlgo hAlgo, const char *pKey, void *pParameter);

/*!
@ingroup SdkApi
@brief   get a pointer on algorithm's control structure

@param hAlgo Algorithms handle instance
@return Pointer on algorithm's control structure
*/
void *acAlgoGetControlPtr(acAlgo hAlgo);


/*!
@ingroup SdkApi
@brief   set an control algorithm parameter

An algorithm has could have some control parameters.
The application can use these processing parameters for its own usage (data presentation or routing ....)
But sometime, an algorithm wants a response to continue its job, and the algorithm check a parameter to clear a state or not.
This function allows to set a control variable using a key and a variable parameter.

You can enumerate all control parameters and the type accepted using dump functions.


@param hAlgo Algorithms handle instance
@param pKey  parameter name
@param ... opaque parameter
@return 0 if success or an error code
*/
int32_t acAlgoSetControl(acAlgo hAlgo, const char *pKey, ...);




/*!
@ingroup SdkApi
@brief   Get an common algorithm  parameter

This function is similar to acAlgoGetControl, but rather to return directly the variable itself, it return the address of the variable.
This function is mainly used for the optimization and avoid the iteration tor return the variable. using this function, the iteration will be done only once
Notice: This function could be used when an algorithm  reuses the same structure for each call to the control

@code
// example
static uint8_t *pCommandRestart = NULL;
uint8_t commandRestart;
if(pMyVariable  == NULL)
{
acAlgoGetControl(hAlgo, "commandRestart",(void *)&pCommandRestart);
}
uint8_t  commandRestart = *pCommandRestart;
@endcode

@param hAlgo Algorithms handle instance
@param pKey  Key Parameter name
@param pParameter opaque pointer
@return 0 if success or an error code
*/
int32_t acAlgoGetControlVarPtr(acAlgo hAlgo, const char *pKey, void **pParameter);



/*!
@ingroup SdkApi
@brief Dump the algorithm properties on the console log
@see  acAlgoSetConfig acAlgoSetCommonConfig
@param hAlgo the algo instance
@return Error code, 0 if success
*/
int32_t acAlgoDump(acAlgo hAlgo);

/*!
@ingroup SdkApi
@brief Set a common algo  parameter

An algorithm has a set of parameters and a common parameter.
Variable parameters depend on the algorithms itself.
Common parameters are present in all algorithms.

Common parameters allow to customize the default algorithms behaviour.
You can for example set an algorithm callback to export data processing or enable/disable features.
The list of parameters is available using algorithm dump functions.
Common parameters cannot be modified when the pipe is started.

This function allows to set a common parameter content.


@see    acAlgoGetCommonConfig
@param hAlgo Algo Instance
@param pKey  Key parameter name
@param pValue   pointer on the variable parameter according to its type
@return Error code, 0 if success
*/
int32_t acAlgoSetCommonConfig(acAlgo hAlgo, const char *pKey, const void *pValue);



/*!
@ingroup SdkApi
@brief Get a common algo  parameter

An algorithm has a set of parameters and a common parameter.
Variable parameters depend on the algorithms itself.
Common parameters are present in all algorithms.

Common parameters allow to customize the default algorithms behaviour.
You can for example set an algorithm callback to export data processing or enable/disable features.
The list of parameters is available using algorithm dump functions.

This function allows to retrieve a common parameter content.



@see    acAlgoSetCommonConfig
@param hAlgo Algo Instance
@param pKey  Parameter name
@param pValue pointer of variable receiving the parameter
@return Error code, 0 if success
*/
int32_t acAlgoGetCommonConfig(acAlgo hAlgo, const char *pKey, void *pValue);

/*!
@ingroup SdkApi
@brief Create a chunk instance

A chunk is a object carrying  the dataflow, a chunk connects two algorithms or connect an algorism to the external hardware
if the chunk's name is a system IOsuch has SysInChunk1, the audio dataflow will be  routed to a pre-build hardware sink,
If the chunk name doesn't match a system io name, the function will create a custom chunk.

A chunk has parameters. When a custom chunk is created, audio chain fills parameters with its default values
According to the chunk, these values can be modified, you can modify all custom chunk parameters, but system IO parameters are read only.
Chunks cannot be modified when the pipe is started
If you modify a system chunk or a custom chunk while the pipe is started , the function will return an error

You can get the description of all chunk parameters using the acChunkDump(hChunk)

@see acChunkDelete
@param hPipe        Pipe instance
@param pChunkName   Pipe Name
@param pphChunk     Pointer receiving  the handle
@return Error code, 0 if success
*/
int32_t acChunkCreate(acPipe hPipe, char *pChunkName, acChunk *pphChunk);

/*!
@ingroup SdkApi
@brief  Delete a chunk

The chunk deletion is not mandatory, because when the pipe will be stopped or if the
pipe starts with an error; all algorism and chunk instances will be automatically deleted.
It is impossible to delete a system io chunk.

@see acChunkCreate
@param hChunk  the chunk instance
@return Error code, 0 if success
*/
int32_t acChunkDelete(acChunk hChunk);

/*!
@ingroup SdkApi
@brief Dump the chunk properties on the console log

@param hChunk chunk instance
@return Error code, 0 if success
*/
int32_t acChunkDump(acChunk hChunk);


/*!
@ingroup SdkApi
@brief Set a chunk parameter configuration

A chunk has a set of parameters, you can set these parameters using this function.
The setting works with the method key value. To set a parameter, you need to know its name and its scope.
A parameter value can have several types, but it is always passed as a string.
It could be an integer, a float, or a flag name, etc..

You can enumerate all parameter descriptions on the console using acAlgoChunk().
You can use this function when the pipe is not running only.
You cannot modify chunk read only parameter such as system chunks.



@see acAlgoChunk
@param hChunk Chunk instance
@param pKey   Key parameter name
@param pValue Parameter value
@return Error code, 0 if success
*/
int32_t acChunkSetConfig(acChunk hChunk, const char *pKey, const char *pValue);


/*!
@ingroup SdkApi
@brief Connect an algo pin out to a chunk

This function is typically used to connect a sink.
For example, an algorithm connected to a system out chunk.

Notice, a connection between 2 algorithms is reciprocal.
ie: if an algo A and B must be connected to a chunk C.

You must create a connection algorithm A and pin Out to the chunk C.
and a connection algorithm B pin In to the chunk C, this job is typically done by acAlgoConnect().




@see acAlgoConnectPinIn acAlgoConnect
@param hPipe Pipe instance
@param hAlgo Algo instance
@param hChunk Chunk instance
@param pinID Algo pinID, the first pin is 0 the second one is 1 etc...
@return Error code, 0 if success
*/
int32_t acPipeConnectPinOut(acPipe hPipe, acAlgo hAlgo, uint32_t pinID, acChunk hChunk);


/*!
@ingroup SdkApi
@brief Connect an algo pin In to a chunk

This function is typically used to connect a source.
For example, an algorithm connected to a system in chunk.

Notice, a connection between 2 algorithms is reciprocal.
ie: if an algo A and B must be connected to a chunk C.

You must create a connection algorithm A and pin Out to the chunk C.
and a connection algorithm B pin In to the chunk C, this job is typically done by acAlgoConnect().


@see acAlgoConnectPinOut acAlgoConnect
@param hPipe Pipe instance
@param hAlgo Algo instance
@param hChunk Chunk instance
@param pinID Algo pinID, the first pin is 0 the second one is 1 etc...

@return Error code, 0 if success
*/
int32_t acPipeConnectPinIn(acPipe hPipe, acAlgo hAlgo, uint32_t pinID, acChunk hChunk);


/*!
@ingroup SdkApi
@brief Connect an two algos with a chunk
This function is typically used to connect to algo between them .

@see acAlgoConnectPinOut acAlgoConnectPinIn
@param hPipe Pipe instance
@param hAlgoSrc Source algo instance
@param srcPinID Algo pinID, the first pin is 0 the second one is 1 etc...
@param hAlgoDst Destination  algo instance
@param hChunk Chunk instance
@param dstPinID Algo pinID, the first pin is 0 the second one is 1 etc...
@return Error code, 0 if success
*/
int32_t acPipeConnect(acPipe hPipe, acAlgo hAlgoSrc, uint32_t srcPinID, acChunk hChunk, acAlgo hAlgoDst, uint32_t dstPinID);

/**
@ingroup SdkApi
@brief Start or Stop the pipeline

When the pipe is formed, you can start and stop the pipe.
if the start fails, the console log reports all information you need to fix the issue.

@param hPipe Pipe instance
@param state state AC_START or AC_STOP
@return Error code, 0 if success
*/
int32_t acPipePlay(acPipe hPipe, acState_t state);



/**
@ingroup SdkApi
@brief return 0 if the pipe is playing
this function will return an error  if the pipe is not started due to an error or if the graph is nit set

@param hPipe Pipe instance
@return Error code, 0 if success
*/
int32_t acPipeIsPlaying(acPipe hPipe);


/*!
@ingroup SdkApi
@brief Print a formatted trace on the console log

The functions produces output according to a format similar to the function printf()

@param pFormat  format string
*/
void acTrace(const char *pFormat, ...);

/*!
@ingroup SdkApi
@brief Set an environmental parameter

This function allows to modify global behaviour, such as the trace level au other settings.
You can get the description of all environmental parameters using the acEnvDump().

@see acEnvDump
@param pKey  Key string
@return Error code, 0 if success
*/
int32_t acEnvSetConfig(const char *pKey, ...);



/*!
@ingroup SdkApi
@brief Get an environmental parameter

This function returns an environmental parameter.
notice: parameters could be ï¿½read-onlyï¿½ and return an error.



@see acEnvDump
@param pKey  Key string
@param pParam  Parameter value
@return Error code, 0 if success
*/
int32_t acEnvGetConfig(const char *pKey, void *pParam);

/*!
@ingroup SdkApi
@brief Get an environmental descriptor

This function returns an environmental descriptor with all environmental data.



@return environment data descriptor
*/
void *acEnvGetDescriptor(void);



/*!
@ingroup SdkApi
@brief Dump all environmental parameters

Prints the description and the value expected.

@see acEnvSetConfig
@return 0 if success or an error code
*/
int32_t acEnvDump(void);


/*!
@ingroup SdkApi
@brief Dump algos cycle counts

This function works only if the pipe is running.
Each running algo will expose various CPU load consumption.

@param traceLog          flag telling if cpu load values must be displayed on terminal
@param interruptsOnly    flag telling if only interrupts cpu load must be computed
@param pInterruptsPcent  pointer to returned interrupts cpu load value in pcent
@param pInterruptsMHz    pointer to returned interrupts cpu load value in MHz
@see acEnvSetConfig
@return 0 if success or an error code
*/
int32_t acCyclesDump(bool traceLog, bool interruptsOnly, float *pInterruptsPcent, float *pInterruptsMHz);


/*!
@ingroup SdkApi
@brief Dump algos memory usage

@param verbose          detailed memory usage

@return 0 if success or an error code
*/
int32_t acAlgosMemDump(bool const verbose);


/**
* @brief  Check status from error code
*         check if status is OK: neither warning, nor error
@param error  error code
@return 0 if success or an error code
*/
int32_t acErrorIsOk(int32_t error);


/**
* @brief  Check status from error code
*         check if status is NOK: either warning or error
@param error  error code
@return 0 if success or an error code
*/
int32_t acErrorIsNok(int32_t error);


/**
* @brief  Check status from error code
*         check if status is warning: neither OK, nor error
@param error  error code
@return 0 if success or an error code
*/
int32_t acErrorIsWarning(int32_t error);


/**
* @brief  Check status from error code
*         check if status is error: neither OK, nor warning
@param error  error code
@return 0 if success or an error code
*/
int32_t acErrorIsError(int32_t error);


/**
* @brief  Check status from error code
*         check if status is no error: either OK or warning
@param error  error code
@return 0 if success or an error code
*/
int32_t acErrorIsNoError(int32_t error);

/**
* @brief  Weak function that allows to manage persistent configuration from AcSDK
*
@param  audio configuration identifying string
@return index of persistent configuration
*/
int32_t acGetPersistConfigIndexFromString(const char *pId);

/**
* @brief  Weak function that allows to manage persistent configuration from AcSDK
*
@param  audio configuration index
@return None
*/
void acSetPersistSetConfigIndex(int32_t index);

/**
* @brief  Ac Hook : Graph creation
*/

void AudioChainInstance_initGraph(void);

/**
* @brief  Ac Hook : set the default configuration
  This function is called at the very beginning of the system initialization.
  It gives the opportunity to replace the default audio config by the configuration used during the code generation.

*/

void AudioChainInstance_initSystem(void);




#ifdef __cplusplus
}
#endif


#endif /*_acSDK_*/

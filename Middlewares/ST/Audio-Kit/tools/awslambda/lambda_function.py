
##############################################################################
# @file    buildLambda.sh
# @author  MCD Application Team
# @version V1.0.0
# @date    2/28/2019
# @brief   Implements an aws serverless lambda function
#		   This function exposes an API for audio chaine, the name of the is int the json field api and parameters in the json field params
#          for the now, only fBeamforming is implemented
##############################################################################
# @attention
#
# <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
# All rights reserved.</center></h2>
#
# This software component is licensed by ST under Ultimate Liberty license
# SLA0044, the "License"; You may not use this file except in compliance with
# the License. You may obtain a copy of the License at:
#                             www.st.com/SLA0044
#
##############################################################################



import json
import io
import matplotlib.pyplot as plt
import base64
from PIL import Image
from fbeamforming.src.scripts.algo import fBeamforming as Beamformer
bDebug = False

# class API fBeamforming
class bfLambda:

    def __init__(self):
        self.algo = None
        pass
    # load the params request and fill it at the beam former format
    def loadRequest(self, event=None):
        self.params ={}
        for i in event["params"]:
            if hasattr(Beamformer, i):
                key = getattr(Beamformer, i)
                value = event["params"][i]
            else:
                key = i
                value = event["params"][i];
            self.params[key] = value

    # compute the the algo and transform params in coefs
    def compute(self):
        self.algo = Beamformer('Beamformer', params=self.params, verbose=True)
        self.algo.compute_algo()

    # just a write for debug
    def write(self,name,data):
        asc = json.dumps(data);
        with open(name, 'w') as f:
            f.write(asc)
    # return the response as a JSON string
    def getResponse(self, bImage=False):
        response = {}
        self.algo.write_json(response)
#        self.write("test.result",response)
        #if bImage is set, we compute the polarPlot  as a PNG base64 and it to the json image field
        if bImage == True:
            #if bDebug we use the QT representation for debug
            if bDebug == True:
                if self.params["plot_freq"] == -1:
                    self.algo.polar_plot_all()
                else:
                    self.algo.polar_plot_val(self.params["plot_freq"] )

            else:
                # create the polarplot without QT
                if  self.params["plot_freq"]  == -1:
                    self.algo.polar_plot_all(True)
                else:
                    self.algo.polar_plot_val( self.params["plot_freq"] ,True)
                # reduce the image  weight and compress it as jpeg in a buffer
                buf = io.BytesIO();
                plt.savefig(buf, format='png')
                buf.seek(0)
                image = Image.open(buf)
                new_image = image.resize((512, 512))
                new_image = new_image.convert("RGB");
                bufFinal = io.BytesIO();
                new_image.save(bufFinal, format='jpeg', quality=70)
                #encode the result in Base64
                encoded_string = base64.b64encode(bufFinal.getbuffer())
                buf.seek(0)
#                with open("output.jpg", "wb") as f:
#                    f.write(bufFinal.getbuffer())
                #Add the image to the json
                response["image"] = encoded_string.decode('ascii');
#       convert the json in string
        return json.dumps(response)


#main entry aws entrypoint
def lambda_handler(event, context):
    global batchFilePath
#    print(event)
    #get the message body ( ie the json ) */
    str = event["body"];
    body = json.loads(str)
#    print(body)
    errorCode = 404
    result = "Service error"
    # check the API service
    if body["api"] == "fbeamforming":
        #instantiate the beanforming function
        function = bfLambda()
        function.loadRequest(body)
        # compute ....
        function.compute()
        #compute the image if requested by the caller
        bImage = "withImage" in body
        result = function.getResponse(bImage)
        errorCode = 200
    #log only on demand
    if "log" in body:
        print("Received event: " + json.dumps(body))
        print("Result event: " + result)
    #return the result
    return  {"statusCode": errorCode, "body": result}


# uncomment these lines to test with pycharm
if __name__ == "__main__":
    strReq = '{"withImage":true,"api":"fBeamforming","params":{"type":"SIR","plot_freq":8000,"sample_freq":16000,"mic_nb":2,"bands_nb":129,"mic_dist":10,"noise_std":0.000001,"speech_sources":[0],"interference_sources":[20],"antenna_id":0}}'
    request = {"body":strReq}
    lambda_handler(request,0);

package iot.unipi.it;

import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapHandler;
import org.eclipse.californium.core.CoapObserveRelation;
import org.eclipse.californium.core.CoapResponse;
import org.eclipse.californium.core.coap.CoAP.Code;
import org.eclipse.californium.core.coap.MediaTypeRegistry;
import org.eclipse.californium.core.coap.Request;

import iot.unipi.it.Utils;

public class CoapHandler{
	private CoapClient client_FireDetectorSensor;
	private CoapClient client_OxygenControllerSensor;
	private float oxygen_target;

	private static CoapHandler instance = null;

    public static CoapHandler getInstance(float oxygen_target) {
        if (instance == null)
            instance = new CoapHandler(oxygen_target);
        return instance;
	}

	public void addFireDetector(String ipAddress){
		client_FireDetectorSensor = new CoapClient("coap://[" + ipAddress + "]/fire_detector");
		client_FireDetectorSensor.observe(
			new CoapHandler() {
                    public void onLoad(CoapResponse response) {
                        handleFireAlarm(response);
                    }

                    public void onError() {
                        System.err.println("OBSERVING FAILED");
                    }
			});
	}

	public void handleFireAlarm(CoapResponse response){
		try{
			String responseText = response.getResponseText();
			Map<String, Object> jsonObject = Utils.jsonParser(requestText);
			if((boolean)jsonObject.get("fire_detected")){
				//TODO: invia a sensore ossigeno di abbassare velocemente
			}
		} catch (Exception e){
			System.err.println("The message received was not valid");
		}
	}
}
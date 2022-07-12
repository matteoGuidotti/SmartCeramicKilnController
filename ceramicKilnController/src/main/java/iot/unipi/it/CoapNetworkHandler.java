package iot.unipi.it;

import java.util.Map;

import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapHandler;
import org.eclipse.californium.core.CoapObserveRelation;
import org.eclipse.californium.core.CoapResponse;
import org.eclipse.californium.core.coap.CoAP.Code;
import org.eclipse.californium.core.coap.MediaTypeRegistry;
import org.eclipse.californium.core.coap.Request;

import iot.unipi.it.Utils;

public class CoapNetworkHandler{

	public enum Controller_mode{
		UP_SLOW,
		UP_FAST,
		DOWN_SLOW,
		DOWN_UP,
		OFF;
	}

	private CoapClient client_FireDetectorSensor;
	private CoapClient client_OxygenControllerSensor;
	private double oxygen_target;
	private double acceptableRange = 1;
	private Controller_mode controllerMode = Controller_mode.OFF;

	private static CoapNetworkHandler instance = null;

	public CoapNetworkHandler(float oxygen_target){
		this.oxygen_target = oxygen_target;
	}

    public static CoapNetworkHandler getInstance(float oxygen_target) {
        if (instance == null)
            instance = new CoapNetworkHandler(oxygen_target);
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
			Map<String, Object> jsonResponse = Utils.jsonParser(responseText);
			if((Boolean)jsonResponse.get("fire_detected")){
				//TODO: invia a sensore ossigeno di abbassare velocemente
			}
		} catch (Exception e){
			System.err.println("The message received was not valid");
		}
	}

	public void addOxygenController(String ipAddress){
		client_OxygenControllerSensor = new CoapClient("coap://[" + ipAddress + "]/oxygen_sensor");
		client_OxygenControllerSensor.observe(
			new CoapHandler() {
				public void onLoad(CoapResponse response) {
					handleOxygenChange(response);
				}

				public void onError() {
					System.err.println("OBSERVING FAILED");
				}	
			}
		);
	}

	public void handleOxygenChange(CoapResponse response){
		String responseText = response.getResponseText();
		Map<String, Object> jsonResponse = Utils.jsonParser(responseText);
		double new_oxygenLevel = (Double)jsonResponse.get("oxygen_sensor");
		//TODO: controllare il livello di ossigeno rispetto al (target +/- acceptableRange) e attuare di conseguenza
	}
}
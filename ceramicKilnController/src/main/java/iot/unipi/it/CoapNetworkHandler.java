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
		DOWN_FAST,
		DOWN_SLOW,
		OFF;
	}

	private CoapClient[] client_FireDetectorSensor;
	private CoapClient client_OxygenControllerSensor;
	private double oxygen_target;
	private double acceptableRange;
	private Controller_mode controllerMode = Controller_mode.OFF;
	private String[] fireDetectorAddress = new String[2];
	private String oxygenControllerAddress;

	private static CoapNetworkHandler instance = null;

	public CoapNetworkHandler(double oxygen_target, double acceptableRange){
		this.oxygen_target = oxygen_target;
		this.acceptableRange = acceptableRange;
		fireDetectorAddress[0] = null;
		fireDetectorAddress[1] = null;
		oxygenControllerAddress = null;
	}

    public static CoapNetworkHandler getInstance(double oxygen_target, double acceptableRange) {
        if (instance == null)
            instance = new CoapNetworkHandler(oxygen_target, acceptableRange);
        return instance;
	}

	public void addFireDetector(String ipAddress){
		int index = 0;
		if(fireDetectorAddress[0] != null)
			index = 1;
		fireDetectorAddress[index] = ipAddress;
		client_FireDetectorSensor[index] = new CoapClient("coap://[" + ipAddress + "]/fire_detector");
		client_FireDetectorSensor[index].observe(
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
			CoapResponse postResponse;
			Map<String, Object> jsonResponse = Utils.jsonParser(responseText);
			if((Boolean)jsonResponse.get("fire_detected")){
				//TODO: invia a sensore ossigeno di abbassare velocemente
				DbUtility.insertFireAlarm(true);
				CoapClient postClient = new CoapClient("coap://[" + oxygenControllerAddress + "]/oxygen_sensor?type=filter&cause=FIRE&mode=on");
				postClient.post("", MediaTypeRegistry.TEXT_PLAIN);
				System.out.println("Requests to switch on the oxygen filter [mode FAST] sent");
				postClient.shutdown();
				controllerMode = Controller_mode.DOWN_FAST;
			}
		} catch (Exception e){
			System.err.println("The message received was not valid");
		}
	}

	public void addOxygenController(String ipAddress){
		oxygenControllerAddress = ipAddress;
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
		DbUtility.insertOxygen(new_oxygenLevel);
		if(controllerMode == Controller_mode.DOWN_FAST){
			//we are trying to reduce a fire
			if(new_oxygenLevel <= 10){
				//this level of oxygen does not enable to the flames to rise
				System.out.println("Sending stop alarm messages");
				CoapClient postClient = new CoapClient("coap://[" + oxygenControllerAddress + "]/oxygen_sensor?type=filter&cause=FIRE&mode=off");
				postClient.post("", MediaTypeRegistry.TEXT_PLAIN);
				System.out.println("Request to switch off the oxygen filter sent");
				postClient.shutdown();
				//sending to the fire_detector sensors the stop alarm messages
				for(int i = 0; i < 2; i++){
					postClient = new CoapClient("coap://[" + fireDetectorAddress[i] + "]/fire_detector?alarm=off");
					postClient.post("", MediaTypeRegistry.TEXT_PLAIN);
					System.out.println("Request to switch off the alarm sent");
					postClient.shutdown();
				}
				controllerMode = Controller_mode.OFF;
			}
		}
		else if(controllerMode == Controller_mode.UP_FAST){
			//we are trying to enter in the kiln
			if(new_oxygenLevel >= 21){
				//this level of oxygen is sufficient to consent to humans to live well
				System.out.println("Sending stop oxygen emitter messages");
				CoapClient postClient = new CoapClient("coap://[" + oxygenControllerAddress + "]/oxygen_sensor?type=emitter&cause=ADMIN&mode=off");
				postClient.post("", MediaTypeRegistry.TEXT_PLAIN);
				System.out.println("Request to switch off the oxygen emitter sent");
				postClient.shutdown();
				controllerMode = Controller_mode.OFF;
			}
		}
		else{
			//we are in a normal situation, in which the oxygen level has to be as near as possible to the target
			if(new_oxygenLevel > oxygen_target + acceptableRange && controllerMode != Controller_mode.DOWN_SLOW){
				controllerMode = Controller_mode.DOWN_SLOW;
				CoapClient postClient = new CoapClient("coap://[" + oxygenControllerAddress + "]/oxygen_sensor?type=filter&cause=CTRL&mode=on");
				postClient.post("", MediaTypeRegistry.TEXT_PLAIN);
				System.out.println("Request to switch on the oxygen filter [mode SLOW] sent");
				postClient.shutdown();
			}
			else if(new_oxygenLevel < oxygen_target - acceptableRange && controllerMode != Controller_mode.UP_SLOW){
				controllerMode = Controller_mode.UP_SLOW;
				CoapClient postClient = new CoapClient("coap://[" + oxygenControllerAddress + "]/oxygen_sensor?type=emitter&cause=CTRL&mode=on");
				postClient.post("", MediaTypeRegistry.TEXT_PLAIN);
				System.out.println("Request to switch on the oxygen emitter [mode FAST] sent");
				postClient.shutdown();
			}
		}
	}

	public void changeTargetOxygen(double target){
		oxygen_target = target;
	}

	public void changeAccOxygen(double range){
		acceptableRange = range;
	}
}
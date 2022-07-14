package iot.unipi.it;

import java.util.HashMap;
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

	private final String JSON_OX_EMITTER_SLOW = "{\"type\":\"emitter\", \"cause\": \"ADMIN\", \"mode\":\"on\"}";
	private final String JSON_OX_EMITTER_FAST = "{\"type\":\"emitter\", \"cause\": \"CTRL\", \"mode\":\"on\"}";
	private final String JSON_OX_FILTER_FAST = "{\"type\":\"filter\", \"cause\": \"ADMIN\", \"mode\":\"on\"}";
	private final String JSON_OX_FILTER_SLOW = "{\"type\":\"filter\", \"cause\": \"CTRL\", \"mode\":\"on\"}";
	private final String JSON_OX_EMITTER_OFF = "{\"type\":\"emitter\", \"cause\": \"ADMIN\", \"mode\":\"off\"}";
	private final String JSON_OX_FILTER_OFF = "{\"type\":\"filter\", \"cause\": \"ADMIN\", \"mode\":\"off\"}";
	private final String JSON_STOP_ALARM = "{\"alarm\":\"stop\"}";
	private final String JSON_START_ALARM = "{\"alarm\":\"start\"}";

	private CoapClient[] client_FireDetectorSensor = new CoapClient[2];
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
		System.out.println("The ipAddress of the registering sensor is " + ipAddress);
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
			String responseText = response.getResponseText();
			CoapResponse postResponse;
			Map<String, Object> jsonResponse = Utils.jsonParser(responseText);
			for(String key: jsonResponse.keySet()){
				System.out.println("chiave json: " + key);
			}
			if((Boolean)jsonResponse.get("fire_detected")){
				//DbUtility.insertFireAlarm(true);
				client_OxygenControllerSensor.post(new CoapHandler() {
					public void onLoad(CoapResponse response) {
						if(response != null){
							if(!response.isSuccess()){
								System.out.println("non success");
							}
						}
					}

					public void onError() {
						System.err.println("OBSERVING FAILED");
					}	
				}, JSON_OX_FILTER_FAST, MediaTypeRegistry.APPLICATION_JSON);
				System.out.println("Request to switch on the oxygen filter [mode FAST] sent");
				controllerMode = Controller_mode.DOWN_FAST;
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
		double new_oxygenLevel = (Double)jsonResponse.get("oxygen_value");
		//DbUtility.insertOxygen(new_oxygenLevel);
		if(controllerMode == Controller_mode.DOWN_FAST){
			//we are trying to reduce a fire
			if(new_oxygenLevel <= 10){
				//this level of oxygen does not enable to the flames to rise
				System.out.println("Sending stop alarm messages");
				client_OxygenControllerSensor.post(new CoapHandler() {
					public void onLoad(CoapResponse response) {
						if(response != null){
							if(!response.isSuccess()){
								System.out.println("non success");
							}
						}
					}

					public void onError() {
						System.err.println("OBSERVING FAILED");
					}	
				}, JSON_OX_FILTER_OFF, MediaTypeRegistry.APPLICATION_JSON);
				System.out.println("Request to switch off the oxygen filter sent");
				//DbUtility.insertFireAlarm(false);
				controllerMode = Controller_mode.OFF;
			}
		}
		else if(controllerMode == Controller_mode.UP_FAST){
			//we are trying to enter in the kiln
			if(new_oxygenLevel >= 21){
				//this level of oxygen is sufficient to consent to humans to live well
				client_OxygenControllerSensor.post(new CoapHandler() {
					public void onLoad(CoapResponse response) {
						if(response != null){
							if(!response.isSuccess()){
								System.out.println("non success");
							}
						}
					}

					public void onError() {
						System.err.println("OBSERVING FAILED");
					}	
				}, JSON_OX_EMITTER_OFF, MediaTypeRegistry.APPLICATION_JSON);
				System.out.println("Request to switch off the oxygen emitter sent");
				controllerMode = Controller_mode.OFF;
			}
		}
		else{
			//we are in a normal situation, in which the oxygen level has to be as near as possible to the target
			if(new_oxygenLevel > oxygen_target + acceptableRange && controllerMode != Controller_mode.DOWN_SLOW){
				controllerMode = Controller_mode.DOWN_SLOW;
				client_OxygenControllerSensor.post(new CoapHandler() {
					public void onLoad(CoapResponse response) {
						if(response != null){
							if(!response.isSuccess()){
								System.out.println("non success");
							}
						}
					}

					public void onError() {
						System.err.println("OBSERVING FAILED");
					}	
				}, JSON_OX_FILTER_SLOW, MediaTypeRegistry.APPLICATION_JSON);
				System.out.println("Request to switch on the oxygen filter [mode SLOW] sent");
			}
			else if(new_oxygenLevel < oxygen_target - acceptableRange && controllerMode != Controller_mode.UP_SLOW){
				controllerMode = Controller_mode.UP_SLOW;
				client_OxygenControllerSensor.post(new CoapHandler() {
					public void onLoad(CoapResponse response) {
						if(response != null){
							if(!response.isSuccess()){
								System.out.println("non success");
							}
						}
					}

					public void onError() {
						System.err.println("OBSERVING FAILED");
					}	
				}, JSON_OX_EMITTER_SLOW, MediaTypeRegistry.APPLICATION_JSON);
				System.out.println("Request to switch on the oxygen emitter [mode SLOW] sent");
			}
		}
	}

	public void changeTargetOxygen(double target){
		oxygen_target = target;
	}

	public void changeAccOxygen(double range){
		acceptableRange = range;
	}

	public void stopFireAlarm(){
		if(controllerMode == Controller_mode.DOWN_FAST){
			client_OxygenControllerSensor.post(new CoapHandler() {
				public void onLoad(CoapResponse response) {
					if(response != null){
						if(!response.isSuccess()){
							System.out.println("non success");
						}
					}
				}

				public void onError() {
					System.err.println("OBSERVING FAILED");
				}	
			}, JSON_OX_FILTER_OFF, MediaTypeRegistry.APPLICATION_JSON);
			System.out.println("Request to switch on the oxygen filter sent");
			controllerMode = Controller_mode.OFF;
		}
		//sending to the fire_detector sensors the stop alarm messages
		for(int i = 0; i < 2; i++){
			if(fireDetectorAddress[i] == null)
				continue;
			client_FireDetectorSensor[i].post(new CoapHandler() {
				public void onLoad(CoapResponse response) {
					if(response != null){
						if(!response.isSuccess()){
							System.out.println("non success");
						}
					}
				}

				public void onError() {
					System.err.println("OBSERVING FAILED");
				}	
			}, JSON_STOP_ALARM, MediaTypeRegistry.APPLICATION_JSON);
			System.out.println("Request to switch off the alarm sent");
		}
		//DbUtility.insertFireAlarm(false);
	}

	public void startFireAlarm(){
		//DbUtility.insertFireAlarm(true);
		client_OxygenControllerSensor.post(new CoapHandler() {
			public void onLoad(CoapResponse response) {
				if(response != null){
					if(!response.isSuccess()){
						System.out.println("non success");
					}
				}
			}

			public void onError() {
				System.err.println("OBSERVING FAILED");
			}	
		}, JSON_OX_FILTER_FAST, MediaTypeRegistry.APPLICATION_JSON);
		System.out.println("Request to switch on the oxygen filter [mode FAST] sent");
		controllerMode = Controller_mode.DOWN_FAST;

		for(int i = 0; i < 2; i++){
			if(fireDetectorAddress[i] == null)
				continue;
			client_FireDetectorSensor[i].post(new CoapHandler() {
				public void onLoad(CoapResponse response) {
					if(response != null){
						if(!response.isSuccess()){
							System.out.println("non success");
						}
					}
				}

				public void onError() {
					System.err.println("OBSERVING FAILED");
				}	
			}, JSON_START_ALARM, MediaTypeRegistry.APPLICATION_JSON);
			System.out.println("Request to switch on the fire alarm sent");
		}
	}

	public void oxygenFill(){
		client_OxygenControllerSensor.post(new CoapHandler() {
			public void onLoad(CoapResponse response) {
				if(response != null){
					if(!response.isSuccess()){
						System.out.println("non success");
					}
				}
			}

			public void onError() {
				System.err.println("OBSERVING FAILED");
			}	
		}, JSON_OX_EMITTER_FAST, MediaTypeRegistry.APPLICATION_JSON);
		System.out.println("Request to switch on the oxygen emitter [mode FAST] sent");
		controllerMode = Controller_mode.UP_FAST;
	}
}

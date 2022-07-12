package iot.unipi.it;

import java.net.SocketException;
import java.nio.charset.StandardCharsets;

import org.eclipse.californium.core.CoapResource;
import org.eclipse.californium.core.CoapServer;
import org.eclipse.californium.core.coap.CoAP.ResponseCode;
import org.eclipse.californium.core.server.resources.CoapExchange;

import iot.unipi.it.Utils;

public class RegistrationServer extends CoapServer {
	private final static CoapHandler coapHandler;
	
	public RegistrationServer(float oxygen_target) throws SocketException {
        this.add(new RegistrationResource());
		coapHandler = CoapHandler.getInstance(oxygen_target);
    }

	class RegistrationResource extends CoapResource{
		public RegistrationResource(){
			super("Registration resource");
		}

		@Override
		public void handlePOST(CoapExchange exchange){
			String requestText = exchange.getRequestText();
			Map<String, Object> jsonObject = Utils.jsonParser(requestText);
			String deviceType = (String) jsonObject.get("deviceType");
			String ipAddress = exchange.getSourceAddress().getHostAddress();

			if(deviceType.equals("fire_detector"))
				coapHandler.addFireDetector(ipAddress);
            else if(deviceType.equals("oxygen_controller"))
                coapHandler.addOxygenController(ipAddress);

			exchange.respond(ResponseCode.CREATED, "Registration Completed!".getBytes(StandardCharsets.UTF_8));
		}
	}
}
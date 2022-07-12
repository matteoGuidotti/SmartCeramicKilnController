package iot.unipi.it;

import org.eclipse.californium.core.CoapServer;

public class Collector extends CoapServer {

	public static void main(String[] args) {
		//TODO: richiedi i valori di ossigeno e temperatura desiderati
		float oxygen_target;
		RegistrationServer rs = new RegistrationServer(oxygen_target);
		rs.start();
	}

}

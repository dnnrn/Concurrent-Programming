package p2p_student_pairing_package;

import java.io.*;
import java.net.*;
import java.util.*;

//teacher is also handled in the main function
public class Peer {
    int peer;
    int n;
    int port;
    Map<Integer, String> studentAddresses;  //needs to be map due to broadcast function. id: "localhost:{6100+i}"
    
    Integer partner = null;
    boolean finished = false;
    Random rand = new Random();
    
    public Peer(int peer, int port, int n, Map<Integer, String> studentAddresses) {
        this.peer = peer;
        this.port = port;
        this.n = n;
        this.studentAddresses = studentAddresses;
    }
    
    void startThread() {
        new Thread(() -> listen()).start();
    }
    
    void listen() {
        try (ServerSocket serverSocket = new ServerSocket(port)) {
            System.out.println("Student #" + peer + " waits on port: " + port);
            while (true) {
                Socket socket = serverSocket.accept(); //waits for student or teacher to connect
                ObjectInputStream in = new ObjectInputStream(socket.getInputStream()); //set up listening for message
                PairingMessage msg = (PairingMessage) in.readObject(); //get message
                finished = processMessage(msg); //process message and check if finished
                socket.close(); //they only need to listen for their message and then finish
                if(finished) { //we get 2n - 1 messages in the end - if we had a timeout, we could achieve n messages, but it wouldn't be as reliable
                    break; 
                }
            }
            System.out.println("Student #" + peer + " has partner : student #" + partner);
        } catch(Exception e) {
            System.err.println(e); //prints the error we get
        }
    }
    
    synchronized boolean processMessage(PairingMessage msg) {
        System.out.println("\nStudent #" + peer + " got message. Message counter: " + msg.messageCounter);
        List<Integer> pairedList = new ArrayList<>(msg.pairedList);
        
        if (msg.pairingType == 1) { //if peer is picked
            pairedList.add(peer);
            List<Integer> availablePartners = setAvailablePartners(pairedList);
            
            if (availablePartners.isEmpty()) { //if there is nobody to partner with
                partner = peer;
                System.out.println("Student #" + peer + " found nobody to pair with, pairing with themselves...");
                System.out.println("PROGRAM FINISHED");
                broadcastFinish(msg.messageCounter);
                return true;
            }

            //pick random partner
            partner = availablePartners.get(rand.nextInt(availablePartners.size()));;
            pairedList.add(partner);
            System.out.println("Student #" + peer + " chose to pair with student #" + partner);
            
            //send message to partner with lonely peer
            sendMessage(partner, new PairingMessage(2, msg.messageCounter + 1, pairedList, peer));
        } else if (msg.pairingType == 2) { //if peer is partnered
            //accept partner
            partner = msg.sender;
            System.out.println("STUDENTS #" + msg.sender + " AND #" + peer + " PAIRED!");

            //final peer receives message
            if (msg.messageCounter >= n) {
                System.out.println("Student " + peer + ": Final message received. Message counter: " + msg.messageCounter);
                System.out.println("PROGRAM FINISHED");
                broadcastFinish(msg.messageCounter);
                return true;
            }
            //if not final, send message to let lonely peer pick partner
            List<Integer> availablePartners = setAvailablePartners(pairedList);
            int chosenId = availablePartners.get(rand.nextInt(availablePartners.size()));
            sendMessage(chosenId, new PairingMessage(1, msg.messageCounter + 1, pairedList, peer));
            System.out.println("Student #" + peer + " let student #" + chosenId + " pick a student to pair with.");
        } else if (msg.pairingType == 3) { //when broadcasted to end program
            return true;
        }
        return false;
    }
    
    void sendMessage(int chosenId, PairingMessage msg) {
        try {
            String address = studentAddresses.get(chosenId); //gets address with key (id), (localhost:6100+i)
            String[] addressParts = address.split(":"); //["localhost", 6100+i]
            String host = addressParts[0];
            int portOut = Integer.parseInt(addressParts[1]);
            Socket socket = new Socket(host, portOut); //create socket at localhost:6100+i
            ObjectOutputStream out = new ObjectOutputStream(socket.getOutputStream()); //tap into its output
            out.writeObject(msg); //send pairing request to output
            out.flush(); //send straight away!
            socket.close();
        } catch(Exception e) {
            System.err.println(e); //prints the error we get
        }
    }

    List<Integer> setAvailablePartners(List<Integer> pairedList) { //setup list of available partners
        List<Integer> availablePartners = new ArrayList<>();
        //go through list of pairs and check for unpaired people
        for (int i = 1; i <= n; i++) {
            if (!pairedList.contains(i)) {
                availablePartners.add(i);
            }
        }
        return availablePartners;
    }
    
    //send message to all waiting threads to finish
    void broadcastFinish(int messageCounter) {
        int localCounter = 0;
        for (Integer receiverId : studentAddresses.keySet()) {
            if (receiverId != peer) {
                localCounter++;
                sendMessage(receiverId, new PairingMessage(3, messageCounter + localCounter, new ArrayList<>(), peer));
            }
        }
    }

    public static void main(String[] args) {
        //set up all variables
        Scanner scanner = new Scanner(System.in);
        System.out.print("Enter total number of students: ");
        int n = scanner.nextInt();
        scanner.close();
        
        //set up all students on localhost with ports 6101+
        Map<Integer, String> studentAddresses = new HashMap<>();
        for (int i = 1; i <= n; i++) {
            studentAddresses.put(i, "localhost:" + (6100 + i));
        }
        //start all threads
        for (int i = 1; i <= n; i++) {
            Peer student = new Peer(i, 6100 + i, n, studentAddresses);
            student.startThread();
        }
        //choose first student
        Random rand = new Random();
        int chosenId = 1+ rand.nextInt(n-1);
        
        PairingMessage msg = new PairingMessage(1, 1, new ArrayList<>(), 0);
        // teacher picks random student
        try {
            String address = studentAddresses.get(chosenId); //gets address with key (id), (localhost:6100+i)
            String[] addressParts = address.split(":"); //["localhost", 6100+i]
            String host = addressParts[0];
            int portOut = Integer.parseInt(addressParts[1]);
            Socket socket = new Socket(host, portOut); //create socket at localhost:6100+i
            ObjectOutputStream out = new ObjectOutputStream(socket.getOutputStream()); //tap into its output
            System.out.println("Teacher let student #" + chosenId + " pick a student to pair with");
            out.writeObject(msg); //send pairing request to output
            out.flush(); //send straight away!
            socket.close();
        } catch(Exception e) {
            System.err.println(e); //prints the error we get
        }
    }
}

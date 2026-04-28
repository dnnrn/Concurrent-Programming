package p2p_student_pairing_package;

import java.io.*;
import java.util.*;

public class PairingMessage implements Serializable {
    
    public int pairingType; //pick (1) or confirm pairing (2). (3) to terminate
    public int messageCounter; //messages sent (teacher’s message is counter 1)
    public List<Integer> pairedList; //students paired
    public int sender; //(teacher uses 0)
    
    public PairingMessage(int pairingType, int messageCounter, List<Integer> pairedList, int sender) {
        this.pairingType = pairingType;
        this.messageCounter = messageCounter;
        this.pairedList = pairedList;
        this.sender = sender;
    }
}

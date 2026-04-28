


public class HungryBirds {
  
static class Plate {
    private int worms;
    private final int REFILL;
    private static final int EMPTY = 0;
    private boolean signal = false; //signal to the parent

    public Plate(int maxWorms) {
        this.REFILL = maxWorms;
        this.worms = maxWorms;
    }

    public synchronized void birdEating(int id) {
        while (worms == EMPTY) {
            if (!signal) { // Only one baby bird should signal the parent
                signal = true;
                System.out.println("Baby bird " + id + " chirp! The plate is empty.");
                notify(); // Notify the parent bird
            }
            try {
                wait(); // Baby birds wait until the plate is refilled
            } catch (InterruptedException e) {}
        }
        
        worms--; // Eat one worm
        System.out.println("Baby bird " + id + " ate a worm. Left: " + worms);
        
        if (worms == EMPTY) {
            signal = false; // Reset signal for next cycle
        }
    }

    public synchronized void refill() {
        while (worms != EMPTY) {
            try {
                wait();
            } catch (InterruptedException e) {}
        }
        worms = REFILL;
        System.out.println("Parent bird refilled " + worms + " worms.");
        notifyAll();
    }
}

static class BabyBird implements Runnable {
    private final Plate plate;
    private final int id;

    public BabyBird(Plate plate, int id) {
        this.plate = plate;
        this.id = id;
    }

    public void run() {
        while (true) {
            try { Thread.sleep(200); } catch (InterruptedException e) {}
            plate.birdEating(id);
        }
    }
}

static class ParentBird implements Runnable {
    private final Plate plate;

    public ParentBird(Plate plate) {
        this.plate = plate;
    }

    public void run() {
        while (true) {
            try { Thread.sleep(200); } catch (InterruptedException e) {}
            plate.refill();
        }
    }
}

public static void main(String[] args) {
    int numBirds = 5;
    int maxWorms = 10;

    Plate plate = new Plate(maxWorms);
    
    for (int i = 0; i < numBirds; i++) {
        new Thread(new BabyBird(plate, i)).start();
    }
    new Thread(new ParentBird(plate)).start();
}
}

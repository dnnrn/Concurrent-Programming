package bearAndBeesPackage;

public class bearAndBees {
    //Fair due to bees being prioritized. Bear checks once, and then waits until full.
    static class HoneyPot {
        int honeyCount = 0;
        int capacity; //the capacity of the honeypot

        public HoneyPot(int capacity) {
            this.capacity = capacity;
        }

        public synchronized void addHoney(int id) throws InterruptedException {
            //if honeypot is filled, wait until bear eats honey, keep waiting when notified if still full
            while (honeyCount == capacity) {
                wait();
            }
            honeyCount++;
            System.out.println("Bee number " + id + " added 1 honey to honeypot: " + honeyCount + "/" + capacity);
            // If current bee filled the honeypot, wake up bear (and in consequence the other bees)
            if (honeyCount == capacity) {
                System.out.println("Bee number " + id + " woke up the bear.");
                notifyAll(); // Wake up bear (and any waiting bees - will check if still full).
            }
        }

        public synchronized void eatHoney() throws InterruptedException {
            while (honeyCount < capacity) {
                wait();
            }
            //wait(1000); //we need this delay, otherwise the notifyAll() from the bees activate them again in any case
            System.out.println("The bear emptied the honeypot. Honeypot fill was: " + honeyCount);
            honeyCount = 0;
            notifyAll();
        }
    }

    static class Bear extends Thread {
        HoneyPot honeyPot;

        public Bear(HoneyPot honeyPot) {
            this.honeyPot = honeyPot;
        }

        @Override
        public void run() {
            try {
                while (true) {
                    honeyPot.eatHoney();
                }
            } catch (InterruptedException e) {
                System.out.println(e);
            }
        }
    }

    static class Bee extends Thread {
        HoneyPot honeyPot;
        int id;

        public Bee(HoneyPot honeyPot, int id) {
            this.honeyPot = honeyPot;
            this.id = id;
        }

        @Override
        public void run() {
            try {
                while (true) {
                    Thread.sleep((int)(Math.random() * 1000)); //make sure a single bee doesn't hog the method
                    honeyPot.addHoney(id);
                }
            } catch (InterruptedException e) {
                System.out.println(e);
            }
        }
    }

    public static void main(String[] args) {
        //int MAX_HONEY = 32;
        int H = 32;
        int capacity = H;
        //int MAX_BEES = 8;
        int n = 8;
        int bees = n;

        HoneyPot honeyPot = new HoneyPot(capacity);
        
        Bear bearThread = new Bear(honeyPot);
        bearThread.start();

        for (int i = 0; i <= bees; i++) {
            Bee beeThread = new Bee(honeyPot, i);
            beeThread.start();
        }
    }
}
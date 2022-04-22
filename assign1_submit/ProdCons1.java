/* Author: Mina Vu
 * File:   ProdCons1.java
 * About:  1 producer and 1 consumer program
 */
import java.util.LinkedList;
import java.util.Queue;

public class ProdCons1 {
    final static int BUFSIZE = 20;
    final static int NUMITEMS = 100;
    final static Object syncher = new Object();
    static Queue<Integer> queue = null;

    static Runnable producer = () -> {
        System.out.println("Producer starting...");

        for (int i = 1; i <= NUMITEMS; ++i) {
            synchronized (syncher) {
                while (queue.size() == BUFSIZE) {
                    try {
                        syncher.wait();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
                queue.add(i);
                syncher.notify();
            }
            System.out.println("Producer added " + i + " (qsz: " + queue.size() + ")");
        }

        System.out.println("Producer ending...");
    };

    static Runnable consumer = () -> {
        System.out.println("Consumer starting...");

        for (int i = 1; i <= NUMITEMS; ++i) {
            int val;
            synchronized (syncher) {
                while (queue.size() == 0) {
                    try {
                        syncher.wait();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
                val = queue.remove();
                syncher.notify();
            }
            System.out.println("Consumer rem'd " + val + " (qsz: " + queue.size() + ")");
        }

        System.out.println("Consumer ending...");
    };

    public static void main(String[] args) {
        queue = new LinkedList<>();
        Thread pThread = new Thread(producer);
        Thread cThread = new Thread(consumer);

        try {
            pThread.start();
            cThread.start();
            pThread.join();
            cThread.join();
        } catch (InterruptedException e) {
            System.err.println(e.getMessage());
        }

        System.out.println("Main: all done!");
    }
}

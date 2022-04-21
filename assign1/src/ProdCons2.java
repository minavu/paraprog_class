import java.util.ArrayList;
import java.util.LinkedList;
import java.util.Queue;

public class ProdCons2 {
    final static int BUFSIZE = 20;
    final static int NUMITEMS = 100;
    final static int ENDSIG = -1;
    final static Object syncher = new Object();
    static Queue<Integer> queue = null;
    static int numCons = 0;
    static int[] counts = null;

    static Runnable producer = () -> {
        System.out.println("Producer starting...");

        for (int i = 1; i <= NUMITEMS; ++i) {
            synchronized (syncher) {
                if (queue.size() == BUFSIZE) {
                    try {
                        syncher.wait();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
                queue.add(i);
                syncher.notifyAll();
            }
            System.out.println("Producer added " + i + " (qsz: " + queue.size() + ")");
        }
        for (int i = 0; i < numCons; ++i) {
            synchronized (syncher) {
                if (queue.size() == BUFSIZE) {
                    try {
                        syncher.wait();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
                queue.add(ENDSIG);
                syncher.notifyAll();
            }
            System.out.println("Producer added an ENDSIG (qsz: " + queue.size() + ")");
        }

        System.out.println("Producer ending...");
    };

    public static void consumer(int k) {
        k += 1;
        System.out.println("Consumer[" + k + "] starting...");
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
                if (val == ENDSIG) {
                    syncher.notifyAll();
                    break;
                }
                ++counts[k-1];
                syncher.notifyAll();
            }
            if (val != ENDSIG) {
                System.out.println("Consumer[" + k + "] rem'd " + val + " (qsz: " + queue.size() + ")");
            } else {
                System.out.println("Consumer[" + k + "] rem'd an ENDSIG (qsz: " + queue.size() + ")");
            }
        }
        System.out.println("Consumer[" + k + "] ending...");
    }

    public static void main(String[] args) {
        if (args.length > 0) {
            numCons = Integer.parseInt(args[0]);
        }
        if (numCons == 0) numCons = 1;
        counts = new int[numCons];

        queue = new LinkedList<>();
        Thread pThread = new Thread(producer);
        ArrayList<Thread> consumers = new ArrayList<>(numCons);

        try {
            pThread.start();
            for (int i = 0; i < numCons; ++i) {
                int k = i;
                consumers.add(i, new Thread(() -> consumer(k)));
                consumers.get(i).start();
            }
            pThread.join();
            for (int i = 0; i < numCons; ++i) {
                consumers.get(i).join();
            }
        } catch (InterruptedException e) {
            System.err.println(e.getMessage());
        }

        int sum = 0;
        System.out.print("Consumer stats: [");
        for (int i = 0; i < numCons; ++i) {
            sum += counts[i];
            System.out.print(counts[i]);
            if (i != numCons -1) {
                System.out.print(",");
            }
        }
        System.out.print("] total = " + sum + "\n");

        System.out.println("Main: all done!");
    }
}

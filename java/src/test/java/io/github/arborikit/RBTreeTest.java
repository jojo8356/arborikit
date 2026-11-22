package io.github.arborikit;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Random;
import java.util.TreeSet;
import org.junit.jupiter.api.Test;

class RBTreeTest {

    @Test
    void insertionEtOrdre() {
        RBTree<Integer> tree = new RBTree<>(50, 30, 70, 20, 40, 60, 80);
        assertEquals(Arrays.asList(20, 30, 40, 50, 60, 70, 80), tree.inorder());
        assertEquals(7, tree.size());
        assertEquals(Integer.valueOf(50), tree.getRoot().getKey());
        assertEquals(RBTree.Color.BLACK, tree.getRoot().getColor());
    }

    @Test
    void invariantsApresInsertions() {
        RBTree<Integer> tree = new RBTree<>();
        for (int value : new int[] {10, 20, 30, 15, 25, 5, 1, 100, 55, 60}) {
            tree.insert(value);
            tree.validate();
        }
    }

    @Test
    void invariantsApresSuppressions() {
        RBTree<Integer> tree = new RBTree<>(50, 30, 70, 20, 40, 60, 80, 10, 25, 35, 45);
        tree.validate();
        for (int value : new int[] {10, 70, 50}) {
            assertTrue(tree.delete(value));
            tree.validate();
        }
        assertEquals(Arrays.asList(20, 25, 30, 35, 40, 45, 60, 80), tree.inorder());
    }

    @Test
    void insertionsTrieesRestentEquilibrees() {
        RBTree<Integer> tree = new RBTree<>();
        for (int i = 1; i <= 20; i++) {
            tree.insert(i);
        }
        tree.validate();
        assertTrue(tree.height() <= 8, "hauteur " + tree.height() + " : équilibrage insuffisant");
        assertEquals(20, tree.size());
        assertTrue(tree.blackHeight() >= 2);
    }

    @Test
    void doublonsEtContains() {
        RBTree<Integer> tree = new RBTree<>(5, 5, 5);
        assertEquals(1, tree.size());
        assertTrue(tree.contains(5));
        assertFalse(tree.contains(6));
        assertFalse(tree.insert(5));
    }

    @Test
    void minMaxSuccesseur() {
        RBTree<Integer> tree = new RBTree<>(50, 30, 70, 20, 60);
        assertEquals(Integer.valueOf(20), tree.min());
        assertEquals(Integer.valueOf(70), tree.max());
        assertEquals(Integer.valueOf(60), tree.successor(50));
        assertEquals(Integer.valueOf(30), tree.predecessor(50));
        assertNull(tree.successor(70));
    }

    @Test
    void couleursExposees() {
        RBTree<Integer> tree = new RBTree<>(50, 30, 70);
        assertNotNull(tree.getRoot());
        List<Integer> counts = tree.colorCounts();
        assertEquals(2, counts.size());
        assertTrue(counts.get(0) + counts.get(1) == 3);
        assertTrue(tree.toTreeString().contains("(N)"));
    }

    @Test
    void stressContreTreeSet() {
        Random random = new Random(7);
        RBTree<Integer> tree = new RBTree<>();
        TreeSet<Integer> reference = new TreeSet<>();

        for (int i = 0; i < 4000; i++) {
            int key = random.nextInt(500);
            if (random.nextBoolean()) {
                tree.insert(key);
                reference.add(key);
            } else {
                tree.delete(key);
                reference.remove(key);
            }
            if (i % 200 == 0) {
                tree.validate();
            }
        }
        tree.validate();
        assertEquals(new ArrayList<>(reference), tree.inorder());
        assertEquals(reference.size(), tree.size());
    }

    @Test
    void arbreVide() {
        RBTree<Integer> tree = new RBTree<>();
        assertTrue(tree.isEmpty());
        assertNull(tree.getRoot());
        assertEquals(-1, tree.height());
        assertFalse(tree.delete(1));
        assertThrows(IllegalStateException.class, tree::min);
    }
}

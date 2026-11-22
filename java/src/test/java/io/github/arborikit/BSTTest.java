package io.github.arborikit;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

import java.util.Arrays;
import java.util.List;
import org.junit.jupiter.api.Test;

class BSTTest {

    @Test
    void insertionEtParcoursTrie() {
        BST<Integer> bst = new BST<>(50, 30, 70, 20, 40, 60, 80);
        assertEquals(Arrays.asList(20, 30, 40, 50, 60, 70, 80), bst.inorder());
        assertEquals(7, bst.size());
        assertEquals(2, bst.height());
        assertEquals(Integer.valueOf(20), bst.min());
        assertEquals(Integer.valueOf(80), bst.max());
        assertTrue(bst.contains(40));
        assertFalse(bst.contains(99));
    }

    @Test
    void doublonsIgnores() {
        BST<Integer> bst = new BST<>();
        assertTrue(bst.insert(5));
        assertFalse(bst.insert(5));
        assertEquals(1, bst.size());
    }

    @Test
    void suppressionTroisCas() {
        BST<Integer> bst = new BST<>(50, 30, 70, 20, 40, 60, 80);

        assertTrue(bst.delete(20)); // feuille
        assertEquals(Arrays.asList(30, 40, 50, 60, 70, 80), bst.inorder());

        assertTrue(bst.delete(30)); // un enfant
        assertEquals(Arrays.asList(40, 50, 60, 70, 80), bst.inorder());

        assertTrue(bst.delete(50)); // deux enfants
        assertEquals(Arrays.asList(40, 60, 70, 80), bst.inorder());
        assertEquals(4, bst.size());

        assertFalse(bst.delete(999));
    }

    @Test
    void successeurEtPredecesseur() {
        BST<Integer> bst = new BST<>(20, 30, 40, 50, 60, 70);
        assertEquals(Integer.valueOf(60), bst.successor(50));
        assertEquals(Integer.valueOf(40), bst.predecessor(50));
        assertNull(bst.successor(70));
        assertNull(bst.predecessor(20));
    }

    @Test
    void parcoursDivers() {
        BST<Integer> bst = new BST<>(50, 30, 70, 20, 40);
        assertEquals(Arrays.asList(50, 30, 20, 40, 70), bst.preorder());
        assertEquals(Arrays.asList(20, 40, 30, 70, 50), bst.postorder());
        assertEquals(Arrays.asList(50, 30, 70, 20, 40), bst.levelorder());
    }

    @Test
    void degenerescenceSurInsertionsTriees() {
        BST<Integer> bst = new BST<>();
        for (int i = 0; i < 10; i++) {
            bst.insert(i);
        }
        assertEquals(9, bst.height());
        assertTrue(bst.isDegenerate());
        List<Integer> expected = Arrays.asList(0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
        assertEquals(expected, bst.inorder());
    }

    @Test
    void arbreVide() {
        BST<Integer> bst = new BST<>();
        assertTrue(bst.isEmpty());
        assertEquals(-1, bst.height());
        assertThrows(IllegalStateException.class, bst::min);
    }

    @Test
    void validateDetecteUneCorruption() {
        BST<Integer> bst = new BST<>(50, 30, 70);
        bst.validate();

        bst.getRoot().getLeft().setKeyForTest(999);
        assertThrows(IllegalStateException.class, bst::validate);
    }

    @Test
    void affichageArborescent() {
        BST<Integer> bst = new BST<>(50, 30, 70);
        String rendered = bst.toTreeString();
        assertTrue(rendered.contains("50"));
        assertTrue(rendered.contains("30"));
        assertTrue(rendered.contains("70"));
    }
}

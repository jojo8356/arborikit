package io.github.arborikit;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

import java.util.Arrays;
import org.junit.jupiter.api.Test;

class GeneralTreeTest {

    @Test
    void structureDeBase() {
        GeneralTree<String> tree = new GeneralTree<>("CEO");
        tree.add("CEO", "VP1");
        tree.add("CEO", "VP2");
        tree.add("VP1", "GM1");
        tree.add("VP1", "GM2");

        assertEquals(5, tree.size());
        assertEquals(2, tree.height());
        assertEquals(Arrays.asList("CEO", "VP1", "GM1", "GM2", "VP2"), tree.preorder());
        assertEquals(Arrays.asList("CEO", "VP1", "VP2", "GM1", "GM2"), tree.levelorder());
        assertEquals(Arrays.asList("GM1", "GM2", "VP2"), tree.leaves());
        assertEquals(Arrays.asList("GM1", "GM2", "VP1", "VP2", "CEO"), tree.postorder());
    }

    @Test
    void profondeurEtChemin() {
        GeneralTree<Integer> tree = new GeneralTree<>(1);
        GeneralTree.Node<Integer> a = tree.getRoot().addChild(2);
        GeneralTree.Node<Integer> b = a.addChild(3);

        assertEquals(2, b.depth());
        assertEquals(Arrays.asList(1, 2, 3), b.pathFromRoot());
        assertTrue(b.isLeaf());
        assertTrue(tree.getRoot().isRoot());
        assertNotNull(tree.find(3));
        assertNull(tree.find(99));
    }

    @Test
    void pruneSupprimeLeSousArbre() {
        GeneralTree<String> tree = new GeneralTree<>("r");
        GeneralTree.Node<String> a = tree.getRoot().addChild("a");
        a.addChild("a1");
        tree.getRoot().addChild("b");

        assertTrue(tree.prune("a"));
        assertEquals(Arrays.asList("r", "b"), tree.preorder());
        assertFalse(tree.prune("r"), "la racine ne se supprime pas");
        assertFalse(tree.prune("inconnu"));
    }

    @Test
    void parentInconnuLeveUneException() {
        GeneralTree<String> tree = new GeneralTree<>("r");
        assertThrows(IllegalArgumentException.class, () -> tree.add("absent", "x"));
    }

    @Test
    void affichageArborescent() {
        GeneralTree<String> tree = new GeneralTree<>("r");
        tree.add("r", "a");
        String rendered = tree.toString();
        assertTrue(rendered.contains("└── r"));
        assertTrue(rendered.contains("a"));
    }
}

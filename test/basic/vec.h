static void vecInstance () {
    Vector v (12);
    TEST_ASSERT_EQUAL(2, v.width()); // 12 bits in 2 bytes
    TEST_ASSERT_EQUAL(0, v.length());
}

static void vecSetAndGrow () {
    Vector v (32);
    TEST_ASSERT_EQUAL(4, v.width());

    v.set(0, 123);
    TEST_ASSERT_EQUAL(1, v.length());
    TEST_ASSERT_EQUAL(123, v.getInt(0));

    v.set(2, 456);
    TEST_ASSERT_EQUAL(3, v.length());
    TEST_ASSERT_EQUAL(123, v.getInt(0));
    TEST_ASSERT_EQUAL(0,   v.getInt(1));
    TEST_ASSERT_EQUAL(456, v.getInt(2));

    v.ins(1, 2);
    TEST_ASSERT_EQUAL(5, v.length());
    TEST_ASSERT_EQUAL(123, v.getInt(0));
    TEST_ASSERT_EQUAL(0,   v.getInt(1));
    TEST_ASSERT_EQUAL(0,   v.getInt(2));
    TEST_ASSERT_EQUAL(0,   v.getInt(3));
    TEST_ASSERT_EQUAL(456, v.getInt(4));

    v.ins(0);
    TEST_ASSERT_EQUAL(6, v.length());
    TEST_ASSERT_EQUAL(0,   v.getInt(0));
    TEST_ASSERT_EQUAL(123, v.getInt(1));
}

static void vecOfValue () {
    VecOf<Value> v;
    TEST_ASSERT_EQUAL(sizeof (Value), v.width());

    v.set(0, 123);
    TEST_ASSERT_EQUAL(1, v.length());
    TEST_ASSERT_EQUAL_INT(123, v.get(0));

    v.set(2, 456);
    TEST_ASSERT_EQUAL(3, v.length());
    TEST_ASSERT_EQUAL_INT(123, v.get(0));
    TEST_ASSERT_EQUAL_INT(0,   v.get(1));
    TEST_ASSERT_EQUAL_INT(456, v.get(2));

    v.ins(1, 2);
    TEST_ASSERT_EQUAL(5, v.length());
    TEST_ASSERT_EQUAL_INT(123, v.get(0));
    TEST_ASSERT_EQUAL_INT(0,   v.get(1));
    TEST_ASSERT_EQUAL_INT(0,   v.get(2));
    TEST_ASSERT_EQUAL_INT(0,   v.get(3));
    TEST_ASSERT_EQUAL_INT(456, v.get(4));

    v.ins(0);
    TEST_ASSERT_EQUAL(6, v.length());
    TEST_ASSERT_EQUAL_INT(0,   v.get(0));
    TEST_ASSERT_EQUAL_INT(123, v.get(1));
}
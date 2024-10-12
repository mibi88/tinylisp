/* A small interpreter for a lisp like language, targetting embedded systems.
 * by Mibi88
 *
 * This software is licensed under the BSD-3-Clause license:
 *
 * Copyright 2024 Mibi88
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* CHANGELOG
 *
 * 2024/10/03: Created file.
 * 2024/10/04: Load functions and added strdef.
 * 2024/10/07: Add print function.
 * 2024/10/08: Use macros when reading from variables and added merge and
 *             numdef functions.
 * 2024/10/09: Parse value in numdef and strdef.
 * 2024/10/12: Fix user function calling bugs. Call user defined function with
 *             arguments. Return value from function.
 */

#include <builtin.h>

#define TL_REGISTER_FUNC(s, parse, f) rc = var_raw_str(&name, s, \
                                                       sizeof(s)-1); \
                                      if(rc) return rc; \
                                      rc = var_builtin_func(&var, f, parse); \
                                      if(rc) return rc; \
                                      rc = tl_add_var(lisp, &var, &name); \
                                      if(rc) return rc

int builtin_register_funcs(TinyLisp *lisp) {
    int rc;
    Var var;
    String name;
    /* strdef */
    TL_REGISTER_FUNC("strdef", 0, builtin_strdef);
    TL_REGISTER_FUNC("numdef", 0, builtin_numdef);
    TL_REGISTER_FUNC("set", 0, builtin_set);
    TL_REGISTER_FUNC("del", 0, builtin_del);
    /* comment */
    TL_REGISTER_FUNC("comment", 0, builtin_comment);
    TL_REGISTER_FUNC("print", 1, builtin_print);
    TL_REGISTER_FUNC("printraw", 0, builtin_printraw);
    TL_REGISTER_FUNC("input", 1, builtin_input);
    TL_REGISTER_FUNC("+", 1, builtin_add);
    TL_REGISTER_FUNC("++", 1, builtin_merge);
    TL_REGISTER_FUNC("params", 0, builtin_params);
    TL_REGISTER_FUNC("list", 1, builtin_list);
    TL_REGISTER_FUNC("fncdef", 0, builtin_fncdef);
    TL_REGISTER_FUNC("defend", 0, builtin_defend);
    TL_REGISTER_FUNC("if", 1, builtin_if);
    TL_REGISTER_FUNC("<", 1, builtin_smaller);
    TL_REGISTER_FUNC(">", 1, builtin_bigger);
    TL_REGISTER_FUNC("<=", 1, builtin_smaller_or_equal);
    TL_REGISTER_FUNC(">=", 1, builtin_bigger_or_equal);
    TL_REGISTER_FUNC("=", 1, builtin_equal);
    TL_REGISTER_FUNC("!=", 1, builtin_not_equal);
    TL_REGISTER_FUNC("-", 1, builtin_substract);
    TL_REGISTER_FUNC("*", 1, builtin_multiply);
    TL_REGISTER_FUNC("/", 1, builtin_divide);
    TL_REGISTER_FUNC("%", 1, builtin_modulo);
    TL_REGISTER_FUNC("floor", 1, builtin_floor);
    TL_REGISTER_FUNC("ceil", 1, builtin_ceil);
    TL_REGISTER_FUNC("parsenum", 1, builtin_parsenum);
    TL_REGISTER_FUNC("callif", 0, builtin_callif);
    /* TODO: numstr: Convert float to string. */
    /* TODO: head and tail */
    return TL_SUCCESS;
}

int builtin_comment(void *_lisp, void *_args, void *_parsed, size_t argnum,
                    void *_returned) {
    int rc;
    TL_UNUSED(_lisp);
    TL_UNUSED(_args);
    TL_UNUSED(_parsed);
    TL_UNUSED(argnum);
    TL_UNUSED(_returned);
    rc = var_str(_returned, "", 0);
    if(rc) return rc;
    return TL_SUCCESS;
}

int builtin_strdef(void *_lisp, void *_args, void *_parsed, size_t argnum,
                   void *_returned) {
    TinyLisp *lisp = _lisp;
    Var *args = _args;
    Var value;
    String name;
    int rc;
    TL_UNUSED(_parsed);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(args[0].type != TL_T_NAME) return TL_ERR_BAD_TYPE;
    if(VAR_LEN(args) != 1) return TL_ERR_INVALID_LIST_SIZE;
    rc = call_parse_arg(lisp, args+1, &value);
    if(rc) return rc;
    if(value.type != TL_T_STR) return TL_ERR_BAD_TYPE;
    rc = var_raw_str(&name, VAR_STR_DATA(VAR_GET_ITEM(args, 0)),
                     VAR_STR_LEN(VAR_GET_ITEM(args, 0)));
    if(rc){
        var_free(&value);
        free(name.data);
        return rc;
    }
    rc = tl_add_var(lisp, &value, &name);
    if(rc){
        var_free(&value);
        free(name.data);
        return rc;
    }
    rc = var_copy(&value, _returned);
    if(rc){
        var_free(&value);
        free(name.data);
        return rc;
    }
    return TL_SUCCESS;
}

int builtin_numdef(void *_lisp, void *_args, void *_parsed, size_t argnum,
                   void *_returned) {
    TinyLisp *lisp = _lisp;
    Var *args = _args;
    Var value;
    String name;
    int rc;
    TL_UNUSED(_parsed);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(args[0].type != TL_T_NAME) return TL_ERR_BAD_TYPE;
    if(VAR_LEN(args) != 1) return TL_ERR_INVALID_LIST_SIZE;
    rc = call_parse_arg(lisp, args+1, &value);
    if(rc) return rc;
    if(value.type != TL_T_NUM) return TL_ERR_BAD_TYPE;
    rc = var_raw_str(&name, VAR_STR_DATA(VAR_GET_ITEM(args, 0)),
                     VAR_STR_LEN(VAR_GET_ITEM(args, 0)));
    if(rc){
        var_free(&value);
        free(name.data);
        return rc;
    }
    rc = tl_add_var(lisp, &value, &name);
    if(rc){
        var_free(&value);
        free(name.data);
        return rc;
    }
    rc = var_copy(&value, _returned);
    if(rc){
        var_free(&value);
        free(name.data);
        return rc;
    }
    return TL_SUCCESS;
}

int builtin_set(void *_lisp, void *_args, void *_parsed, size_t argnum,
                void *_returned) {
    TinyLisp *lisp = _lisp;
    Var *args = _args;
    Var value;
    int rc;
    TL_UNUSED(_parsed);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(args[0].type != TL_T_NAME) return TL_ERR_BAD_TYPE;
    if(VAR_LEN(args) != 1) return TL_ERR_INVALID_LIST_SIZE;
    rc = call_parse_arg(lisp, args+1, &value);
    if(rc) return rc;
    rc = tl_set_var(lisp, &value, &args->items->string);
    if(rc){
        var_free(&value);
        return rc;
    }
    rc = var_free(&value);
    if(rc) return rc;
    rc = var_num_from_float(_returned, 0);
    return rc;
}

int builtin_del(void *_lisp, void *_args, void *_parsed, size_t argnum,
                void *_returned) {
    TinyLisp *lisp = _lisp;
    Var *args = _args;
    int rc;
    TL_UNUSED(_parsed);
    if(argnum < 1) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 1) return TL_ERR_TOO_MANY_ARGS;
    if(args[0].type != TL_T_NAME) return TL_ERR_BAD_TYPE;
    if(VAR_LEN(args) != 1) return TL_ERR_INVALID_LIST_SIZE;
    rc = tl_del_var(lisp, &args->items->string);
    if(rc){
        return rc;
    }
    rc = var_num_from_float(_returned, 0);
    return rc;
}

int builtin_print(void *_lisp, void *_args, void *_parsed, size_t argnum,
                  void *_returned) {
    int rc;
    size_t i;
    Var *args = _parsed;
    TL_UNUSED(_lisp);
    TL_UNUSED(_args);
    if(argnum < 1) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 1) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) < 1){
        puts("()");
        return TL_SUCCESS;
    }
    if(VAR_LEN(args) > 1) fputc('(', stdout);
    for(i=0;i<VAR_LEN(args);i++){
        switch(args[0].type){
            case TL_T_STR:
                if(VAR_LEN(args) > 1) fputc('"', stdout);
                fwrite(VAR_STR_DATA(VAR_GET_ITEM(args, i)), 1,
                       VAR_STR_LEN(VAR_GET_ITEM(args, i)), stdout);
                if(VAR_LEN(args) > 1) fputc('"', stdout);
                if(i < VAR_LEN(args)-1) fputc(' ', stdout);
                break;
            case TL_T_NUM:
                /* TODO: Custom number conversion function. */
                printf("%f", VAR_NUM(VAR_GET_ITEM(args, i)));
                if(i < VAR_LEN(args)-1) fputc(' ', stdout);
                break;
            default:
                return TL_ERR_BAD_TYPE;
        }
    }
    if(VAR_LEN(args) > 1) fputc(')', stdout);
    fputc('\n', stdout);
    rc = var_copy(args, _returned);
    if(rc) return rc;
    return TL_SUCCESS;
}

int builtin_printraw(void *_lisp, void *_args, void *_parsed, size_t argnum,
                     void *_returned) {
    int rc;
    size_t i;
    Var *args = _args;
    TL_UNUSED(_lisp);
    TL_UNUSED(_parsed);
    if(argnum < 1) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 1) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) < 1){
        puts("()");
        return TL_SUCCESS;
    }
    if(VAR_LEN(args) > 1) fputc('(', stdout);
    for(i=0;i<VAR_LEN(args);i++){
        switch(args[0].type){
            case TL_T_STR:
                if(VAR_LEN(args) > 1) fputc('"', stdout);
                fwrite(VAR_STR_DATA(VAR_GET_ITEM(args, i)), 1,
                       VAR_STR_LEN(VAR_GET_ITEM(args, i)), stdout);
                if(VAR_LEN(args) > 1) fputc('"', stdout);
                if(i < VAR_LEN(args)-1) fputc(' ', stdout);
                break;
            case TL_T_NAME:
                fputs("<variable: ", stdout);
                fwrite(VAR_STR_DATA(VAR_GET_ITEM(args, i)), 1,
                       VAR_STR_LEN(VAR_GET_ITEM(args, i)), stdout);
                fputc('>', stdout);
                if(i < VAR_LEN(args)-1) fputc(' ', stdout);
                break;
            case TL_T_NUM:
                /* TODO: Custom number conversion function. */
                printf("%f", VAR_NUM(VAR_GET_ITEM(args, i)));
                if(i < VAR_LEN(args)-1) fputc(' ', stdout);
                break;
            default:
                return TL_ERR_BAD_TYPE;
        }
    }
    if(VAR_LEN(args) > 1) fputc(')', stdout);
    fputc('\n', stdout);
    rc = var_copy(args, _returned);
    if(rc) return rc;
    return TL_SUCCESS;
}

int builtin_input(void *_lisp, void *_args, void *_parsed, size_t argnum,
                  void *_returned) {
    int rc;
    char c;
    Var *args = _parsed;
    TL_UNUSED(_lisp);
    TL_UNUSED(_args);
    if(argnum < 1) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 1) return TL_ERR_TOO_MANY_ARGS;
    else if(args[0].type != TL_T_STR) return TL_ERR_BAD_TYPE;
    if(VAR_LEN(args) != 1) return TL_ERR_INVALID_LIST_SIZE;
    fwrite(VAR_STR_DATA(VAR_GET_ITEM(args, 0)), 1,
           VAR_STR_LEN(VAR_GET_ITEM(args, 0)), stdout);
    var_str(_returned, "", 0);
    while((c = getc(stdin)) != '\n'){
        rc = var_str_add(_returned, &c, 1);
        if(rc){
            var_free(_returned);
            return rc;
        }
    }
    return TL_SUCCESS;
}

int builtin_add(void *_lisp, void *_args, void *_parsed, size_t argnum,
                void *_returned) {
    int rc;
    Var *args = _parsed;
    TL_UNUSED(_lisp);
    TL_UNUSED(_args);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    else if(args[0].type != args[1].type) return TL_ERR_BAD_TYPE;
    if(VAR_LEN(args) != 1) return TL_ERR_INVALID_LIST_SIZE;
    if(VAR_LEN(args+1) != 1) return TL_ERR_INVALID_LIST_SIZE;
    switch(args[0].type){
        case TL_T_STR:
            rc = var_str_concat(_returned, args, args+1);
            if(rc) return rc;
            break;
        case TL_T_NUM:
            rc = var_num_from_float(_returned, VAR_NUM(VAR_GET_ITEM(args, 0))+
                                    VAR_NUM(VAR_GET_ITEM(args+1, 0)));
            if(rc) return rc;
            break;
        default:
            return TL_ERR_BAD_TYPE;
    }
    return TL_SUCCESS;
}

int builtin_merge(void *_lisp, void *_args, void *_parsed, size_t argnum,
                  void *_returned) {
    int rc;
    Var *args = _parsed;
    TL_UNUSED(_lisp);
    TL_UNUSED(_args);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    rc = var_copy(args, _returned);
    if(rc) return rc;
    rc = var_append(args+1, _returned);
    if(rc) return rc;
    return TL_SUCCESS;
}

int builtin_params(void *_lisp, void *_args, void *_parsed, size_t argnum,
                   void *_returned) {
    int rc;
    Var *args = _args;
    size_t i;
    TL_UNUSED(_lisp);
    TL_UNUSED(_parsed);
    if(!argnum){
        ((Var*)_returned)->null = 0;
        ((Var*)_returned)->size = 0;
        ((Var*)_returned)->items = NULL;
        ((Var*)_returned)->type = TL_T_NAME;
        return TL_SUCCESS;
    }
    rc = var_copy(args, _returned);
    if(rc) return rc;
    for(i=1;i<argnum;i++){
        rc = var_append(args+i, _returned);
        if(rc){
            var_free(_returned);
            return rc;
        }
    }
    return TL_SUCCESS;
}

int builtin_list(void *_lisp, void *_args, void *_parsed, size_t argnum,
                 void *_returned) {
    int rc;
    Var *args = _parsed;
    size_t i;
    TL_UNUSED(_lisp);
    TL_UNUSED(_args);
    TL_UNUSED(_parsed);
    if(!argnum){
        ((Var*)_returned)->null = 0;
        ((Var*)_returned)->size = 0;
        ((Var*)_returned)->items = NULL;
        ((Var*)_returned)->type = TL_T_NUM;
        return TL_SUCCESS;
    }
    rc = var_copy(args, _returned);
    if(rc) return rc;
    for(i=1;i<argnum;i++){
        rc = var_append(args+i, _returned);
        if(rc){
            var_free(_returned);
            return rc;
        }
    }
    return TL_SUCCESS;
}

int builtin_fncdef(void *_lisp, void *_args, void *_parsed, size_t argnum,
                   void *_returned) {
    TinyLisp *lisp = _lisp;
    int rc;
    Var *args = _args;
    Var function;
    String name;
    TL_UNUSED(_parsed);
    if(VAR_LEN(args) != 1) return TL_ERR_INVALID_LIST_SIZE;
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(lisp->i+1 >= lisp->sz) return TL_ERR_FNCDEF_NO_END;
    if(lisp->buffer[lisp->i+1] == '\n') lisp->line++;
    rc = var_user_func(&function, lisp->i+1, lisp->line, args+1);
    /*printf("%d\n", lisp->buffer[lisp->i+2]);*/
    if(rc) return rc;
    rc = var_raw_str(&name, VAR_STR_DATA(VAR_GET_ITEM(args, 0)),
                     VAR_STR_LEN(VAR_GET_ITEM(args, 0)));
    if(rc){
        var_free(&function);
        free(name.data);
        return rc;
    }
    rc = tl_add_var(lisp, &function, &name);
    if(rc){
        var_free(&function);
        free(name.data);
        return rc;
    }
    lisp->perform_calls = 0;
    rc = var_num_from_float(_returned, 0);
    return rc;
}

int builtin_defend(void *_lisp, void *_args, void *_parsed, size_t argnum,
                   void *_returned) {
    TinyLisp *lisp = _lisp;
    size_t n;
    int rc;
    TL_UNUSED(_parsed);
    TL_UNUSED(_args);
    if(argnum > 0) return TL_ERR_TOO_MANY_ARGS;
    lisp->perform_calls = 1;
    if(lisp->stack_cur){
        lisp->stack_cur--;
        lisp->i = lisp->stack[lisp->stack_cur].i;
        lisp->line = lisp->stack[lisp->stack_cur].line;
        for(n=0;n<lisp->stack[lisp->stack_cur].argnum;n++){
            var_free(lisp->stack[lisp->stack_cur].args+n);
        }
        free(lisp->stack[lisp->stack_cur].args);
        lisp->stack[lisp->stack_cur].args = NULL;
        var_free(&lisp->stack[lisp->stack_cur].params);
    }else{
        return TL_ERR_INTERNAL;
    }
    /* TODO: Fix return value bug properly. */
    for(n=(signed)lisp->argstack_cur-2 < 0 ? 0 : lisp->argstack_cur-2;
        n<lisp->argstack_cur;n++){
        var_free(lisp->argstack+n);
    }
    if((signed)lisp->argstack_cur-2 < 0) lisp->argstack_cur = 0;
    else lisp->argstack_cur -= 2;
    rc = var_copy(&lisp->last, _returned);
    if(rc) return rc;
    /*printf("%ld, %ld\n", lisp->i, lisp->line);*/
    return TL_SUCCESS;
}

int builtin_if(void *_lisp, void *_args, void *_parsed, size_t argnum,
               void *_returned) {
    Var *args = _parsed;
    int rc;
    TL_UNUSED(_lisp);
    TL_UNUSED(_args);
    if(argnum < 3) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 3) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != TL_T_NUM) return TL_ERR_BAD_TYPE;
    if(args[0].items->num != 0){
        rc = var_copy(args+1, _returned);
        return rc;
    }
    rc = var_copy(args+2, _returned);
    return rc;
}

int builtin_smaller(void *_lisp, void *_args, void *_parsed, size_t argnum,
               void *_returned) {
    Var *args = _parsed;
    int rc;
    TL_UNUSED(_lisp);
    TL_UNUSED(_args);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1 || VAR_LEN(args+1) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != TL_T_NUM || args[1].type != TL_T_NUM){
        return TL_ERR_BAD_TYPE;
    }
    if(args[0].items->num < args[1].items->num){
        rc = var_num_from_float(_returned, 1);
        return rc;
    }
    rc = var_num_from_float(_returned, 0);
    return rc;
}

int builtin_bigger(void *_lisp, void *_args, void *_parsed, size_t argnum,
                   void *_returned) {
    Var *args = _parsed;
    int rc;
    TL_UNUSED(_lisp);
    TL_UNUSED(_args);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1 || VAR_LEN(args+1) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != TL_T_NUM || args[1].type != TL_T_NUM){
        return TL_ERR_BAD_TYPE;
    }
    if(args[0].items->num > args[1].items->num){
        rc = var_num_from_float(_returned, 1);
        return rc;
    }
    rc = var_num_from_float(_returned, 0);
    return rc;
}

int builtin_smaller_or_equal(void *_lisp, void *_args, void *_parsed,
                             size_t argnum, void *_returned) {
    Var *args = _parsed;
    int rc;
    TL_UNUSED(_lisp);
    TL_UNUSED(_args);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1 || VAR_LEN(args+1) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != TL_T_NUM || args[1].type != TL_T_NUM){
        return TL_ERR_BAD_TYPE;
    }
    if(args[0].items->num <= args[1].items->num){
        rc = var_num_from_float(_returned, 1);
        return rc;
    }
    rc = var_num_from_float(_returned, 0);
    return rc;
}

int builtin_bigger_or_equal(void *_lisp, void *_args, void *_parsed,
                            size_t argnum, void *_returned) {
    Var *args = _parsed;
    int rc;
    TL_UNUSED(_lisp);
    TL_UNUSED(_args);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1 || VAR_LEN(args+1) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != TL_T_NUM || args[1].type != TL_T_NUM){
        return TL_ERR_BAD_TYPE;
    }
    if(args[0].items->num >= args[1].items->num){
        rc = var_num_from_float(_returned, 1);
        return rc;
    }
    rc = var_num_from_float(_returned, 0);
    return rc;
}

int builtin_equal(void *_lisp, void *_args, void *_parsed, size_t argnum,
                  void *_returned) {
    Var *args = _parsed;
    int rc;
    TL_UNUSED(_lisp);
    TL_UNUSED(_args);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1 || VAR_LEN(args+1) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != args[1].type){
        return TL_ERR_BAD_TYPE;
    }
    switch(args[0].type){
        case TL_T_STR:
            if(args[0].items->string.len == args[1].items->string.len){
                if(!memcmp(args[0].items->string.data,
                           args[1].items->string.data,
                           args[0].items->string.len)){
                    rc = var_num_from_float(_returned, 1);
                    return rc;
                }
            }
            rc = var_num_from_float(_returned, 0);
            return rc;
        case TL_T_NUM:
            if(args[0].items->num == args[1].items->num){
                rc = var_num_from_float(_returned, 1);
                return rc;
            }
            rc = var_num_from_float(_returned, 0);
            return rc;
        default:
            return TL_ERR_BAD_TYPE;
    }
    return TL_SUCCESS;
}

int builtin_not_equal(void *_lisp, void *_args, void *_parsed, size_t argnum,
                      void *_returned) {
    Var *args = _parsed;
    int rc;
    TL_UNUSED(_lisp);
    TL_UNUSED(_args);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1 || VAR_LEN(args+1) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != args[1].type){
        return TL_ERR_BAD_TYPE;
    }
    switch(args[0].type){
        case TL_T_STR:
            if(args[0].items->string.len == args[1].items->string.len){
                if(!memcmp(args[0].items->string.data,
                           args[1].items->string.data,
                           args[0].items->string.len)){
                    rc = var_num_from_float(_returned, 0);
                    return rc;
                }
            }
            rc = var_num_from_float(_returned, 1);
            return rc;
        case TL_T_NUM:
            if(args[0].items->num == args[1].items->num){
                rc = var_num_from_float(_returned, 0);
                return rc;
            }
            rc = var_num_from_float(_returned, 1);
            return rc;
        default:
            return TL_ERR_BAD_TYPE;
    }
    return TL_SUCCESS;
}

int builtin_substract(void *_lisp, void *_args, void *_parsed, size_t argnum,
                      void *_returned) {
    Var *args = _parsed;
    int rc;
    TL_UNUSED(_lisp);
    TL_UNUSED(_args);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1 || VAR_LEN(args+1) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != TL_T_NUM || args[1].type != TL_T_NUM){
        return TL_ERR_BAD_TYPE;
    }
    rc = var_num_from_float(_returned, args[0].items->num-args[1].items->num);
    return rc;
}

int builtin_multiply(void *_lisp, void *_args, void *_parsed, size_t argnum,
                     void *_returned) {
    Var *args = _parsed;
    int rc;
    TL_UNUSED(_lisp);
    TL_UNUSED(_args);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1 || VAR_LEN(args+1) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != TL_T_NUM || args[1].type != TL_T_NUM){
        return TL_ERR_BAD_TYPE;
    }
    rc = var_num_from_float(_returned, args[0].items->num*args[1].items->num);
    return rc;
}

int builtin_divide(void *_lisp, void *_args, void *_parsed, size_t argnum,
                   void *_returned) {
    Var *args = _parsed;
    int rc;
    TL_UNUSED(_lisp);
    TL_UNUSED(_args);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1 || VAR_LEN(args+1) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != TL_T_NUM || args[1].type != TL_T_NUM){
        return TL_ERR_BAD_TYPE;
    }
    if(args[1].items->num == 0){
        return TL_ERR_DIVISION_BY_ZERO;
    }
    rc = var_num_from_float(_returned, args[0].items->num/args[1].items->num);
    return rc;
}

int builtin_modulo(void *_lisp, void *_args, void *_parsed, size_t argnum,
                   void *_returned) {
    Var *args = _parsed;
    int rc;
    TL_UNUSED(_lisp);
    TL_UNUSED(_args);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 2) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1 || VAR_LEN(args+1) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != TL_T_NUM || args[1].type != TL_T_NUM){
        return TL_ERR_BAD_TYPE;
    }
    if(args[1].items->num == 0){
        return TL_ERR_DIVISION_BY_ZERO;
    }
    rc = var_num_from_float(_returned, fmod(args[0].items->num,
                                            args[1].items->num));
    return rc;
}

int builtin_floor(void *_lisp, void *_args, void *_parsed, size_t argnum,
                  void *_returned) {
    Var *args = _parsed;
    int rc;
    TL_UNUSED(_lisp);
    TL_UNUSED(_args);
    if(argnum < 1) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 1) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != TL_T_NUM){
        return TL_ERR_BAD_TYPE;
    }
    rc = var_num_from_float(_returned, floor(args[0].items->num));
    return rc;
}

int builtin_ceil(void *_lisp, void *_args, void *_parsed, size_t argnum,
                 void *_returned) {
    Var *args = _parsed;
    int rc;
    TL_UNUSED(_lisp);
    TL_UNUSED(_args);
    if(argnum < 1) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 1) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != TL_T_NUM){
        return TL_ERR_BAD_TYPE;
    }
    rc = var_num_from_float(_returned, ceil(args[0].items->num));
    return rc;
}

int builtin_parsenum(void *_lisp, void *_args, void *_parsed, size_t argnum,
                     void *_returned) {
    Var *args = _parsed;
    int rc;
    TL_UNUSED(_lisp);
    TL_UNUSED(_args);
    if(argnum < 1) return TL_ERR_TOO_FEW_ARGS;
    else if(argnum > 1) return TL_ERR_TOO_MANY_ARGS;
    if(VAR_LEN(args) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    if(args[0].type != TL_T_STR){
        return TL_ERR_BAD_TYPE;
    }
    if(!var_isnum(args[0].items->string.data, args[0].items->string.len)){
        return TL_ERR_BAD_INPUT;
    }
    rc = var_num(_returned, args[0].items->string.data,
                 args[0].items->string.len);
    return rc;
}

int builtin_callif(void *_lisp, void *_args, void *_parsed, size_t argnum,
                   void *_returned) {
    TinyLisp *lisp = _lisp;
    Var *args = _args;
    int rc;
    Var condition;
    Call call;
    TL_UNUSED(_parsed);
    if(argnum < 2) return TL_ERR_TOO_FEW_ARGS;
    if(VAR_LEN(args) != 1 || VAR_LEN(args+1) != 1){
        return TL_ERR_INVALID_LIST_SIZE;
    }
    rc = call_parse_arg(lisp, args, &condition);
    if(rc) return rc;
    if(args[1].type != TL_T_NAME || condition.type != TL_T_NUM){
        return TL_ERR_BAD_TYPE;
    }
    if(condition.items->num != 0){
        var_free(&condition);
        /* Perform the call */
        rc = var_call(&call, args[1].items->string.data,
                      args[1].items->string.len);
        if(rc) return rc;
        rc = call_exec(lisp, &call, args+2, argnum-2, _returned);
        var_free_call(&call);
        if(rc) return rc;
    }else{
        var_free(&condition);
        rc = var_num_from_float(_returned, 1);
        return rc;
    }
    return TL_SUCCESS;
}

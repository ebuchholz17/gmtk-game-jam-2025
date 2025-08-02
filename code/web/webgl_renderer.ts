/// <reference path="shaders\test_vertex.ts"/>

class ShaderProgram {
    public vertexShader: WebGLShader = null;
    public fragmentShader: WebGLShader = null;
    public program: WebGLProgram = null;
}

class WebGLRenderer {
    public canvas: HTMLCanvasElement = null;
    public glContext: WebGLRenderingContext = null;

    public memoryView: DataView = null;

    public spriteShader: ShaderProgram = null;
    public spriteVertexBuffer: WebGLBuffer = null;
    public spriteIndexBuffer: WebGLBuffer = null;

    public basic3DShader: ShaderProgram = null;
    public basic3DVertexBuffer: WebGLBuffer = null;
    public basic3DIndexBuffer: WebGLBuffer = null;

    public windowWidth: number = 0;
    public windowHeight: number = 0;

    public textures: WebGLTexture[] = [];
    public textureSizes: [number, number][] = [];

    public numSpritesDrawnThisFrame: number = 0;
    public spriteBatchNumSpritesDrawn: number = 0;
    public spriteBatchSeenTextures: {[key: number]: boolean} = {};
    public spriteBatchTextureList: number[] = []; // order that textures were drawn
}

let wglr: WebGLRenderer = new WebGLRenderer();
let MAX_NUM_SPRITES: number = 10000;

function compileAndLinkShader (gl: WebGLRenderingContext, 
                               vertexShaderSource: string, 
                               fragmentShaderSource: string): ShaderProgram 
{
    var result = new ShaderProgram();

    result.vertexShader = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(result.vertexShader, vertexShaderSource);
    gl.compileShader(result.vertexShader);
    if (!gl.getShaderParameter(result.vertexShader, gl.COMPILE_STATUS)) {
        console.log(gl.getShaderInfoLog(result.vertexShader));
        return;
    }

    result.fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(result.fragmentShader, fragmentShaderSource);
    gl.compileShader(result.fragmentShader);
    if (!gl.getShaderParameter(result.fragmentShader, gl.COMPILE_STATUS)) {
        console.log(gl.getShaderInfoLog(result.fragmentShader));
        return;
    }

    result.program = gl.createProgram();
    gl.attachShader(result.program, result.vertexShader);
    gl.attachShader(result.program, result.fragmentShader);
    gl.linkProgram(result.program);

    if (!gl.getProgramParameter(result.program, gl.LINK_STATUS)) {
        console.log(gl.getProgramInfoLog(result.program));
        return;
    }

    return result;
}

function createStaticBuffer (gl: WebGLRenderingContext, data: number[]): WebGLBuffer {
    let floatArray = new Float32Array(data);
    let result: WebGLBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, result);
    gl.bufferData(gl.ARRAY_BUFFER, floatArray, gl.STATIC_DRAW);
    return result;
}

function createIndexBuffer (gl: WebGLRenderingContext, data: number[]): WebGLBuffer {
    let uintArray = new Uint16Array(data);
    let result: WebGLBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, result);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, uintArray, gl.STATIC_DRAW);
    return result;
}

function initWebGL (canvas: HTMLCanvasElement, memory: Uint8Array): void {
    wglr.canvas = canvas;
    wglr.glContext = <WebGLRenderingContext>canvas.getContext("webgl", { alpha: false, antialias: false});

    wglr.memoryView = new DataView(memory.buffer);

    let gl: WebGLRenderingContext = wglr.glContext;
    //gl.getExtension("OES_element_index_uint");
    gl.getExtension("OES_standard_derivatives");
    wglr.spriteShader = compileAndLinkShader(gl, sprite_vertex_source, sprite_fragment_source);

    wglr.spriteVertexBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, wglr.spriteVertexBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, 4 * MAX_NUM_SPRITES * 40, gl.STATIC_DRAW);

    let indices: number[] = [];
    for (let spriteIndex = 0; spriteIndex < MAX_NUM_SPRITES; spriteIndex++) {
        indices[spriteIndex * 6 + 0] = (spriteIndex * 4) + 0;
        indices[spriteIndex * 6 + 1] = (spriteIndex * 4) + 2;
        indices[spriteIndex * 6 + 2] = (spriteIndex * 4) + 1;
        indices[spriteIndex * 6 + 3] = (spriteIndex * 4) + 1;
        indices[spriteIndex * 6 + 4] = (spriteIndex * 4) + 2;
        indices[spriteIndex * 6 + 5] = (spriteIndex * 4) + 3;
    }
    wglr.spriteIndexBuffer = createIndexBuffer(gl, indices);

    // 3D
    wglr.basic3DShader = compileAndLinkShader(gl, basic_3d_vertex_source, basic_3d_fragment_source);

    wglr.basic3DVertexBuffer = gl.createBuffer();
    var basic3DData = [
        -1.0, 1.0, 0.0,
        0.0, 0.0,
        //0.0, 0.0, 1.0,

        1.0, 1.0, 0.0,
        1.0, 0.0,
        //0.0, 0.0, 1.0,

        -1.0, -1.0, 0.0,
        0.0, 1.0,
        //0.0, 0.0, 1.0,

        1.0, -1.0, 0.0,
        1.0, 1.0,
        //0.0, 0.0, 1.0,
    ];
    var basic3DDataArray = new Float32Array(basic3DData);

    gl.bindBuffer(gl.ARRAY_BUFFER, wglr.basic3DVertexBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, basic3DDataArray, gl.STATIC_DRAW);

    {
        let indices: number[] = [];
        indices[0] = 0;
        indices[1] = 2;
        indices[2] = 1;
        indices[3] = 1;
        indices[4] = 2;
        indices[5] = 3;
        wglr.basic3DIndexBuffer = createIndexBuffer(gl, indices);
    }
}

function resizeWebGL(windowWidth: number, windowHeight: number): void{
    wglr.windowWidth = windowWidth;
    wglr.windowHeight = windowHeight;
}

function webglOnRenderSpritesStart (): void {
    let gl: WebGLRenderingContext = wglr.glContext;
    gl.viewport(0, 0, wglr.windowWidth, wglr.windowHeight);

    let gameRatio = 356.0 / 200.0;
    let windowRatio = wglr.windowWidth / wglr.windowHeight;

    gl.enable(gl.SCISSOR_TEST);
    if (gameRatio > windowRatio) {
        let viewportHeight = wglr.windowWidth / gameRatio;
        let offset = (wglr.windowHeight - viewportHeight) / 2;
        gl.scissor(0, offset, wglr.windowWidth, viewportHeight);
    }
    else {
        let viewportWidth = wglr.windowHeight * gameRatio;
        let offset = (wglr.windowWidth - viewportWidth) / 2;
        gl.scissor(offset, 0, viewportWidth, wglr.windowHeight);
    }

    gl.disable(gl.DEPTH_TEST);
    gl.disable(gl.CULL_FACE);
    //gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.enable(gl.BLEND);
    gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
}

function webglOnRender3DStart (): void {
    let gl: WebGLRenderingContext = wglr.glContext;

    // NOTE(ebuchholz): hardcoded game size
    let gameRatio = 356.0 / 200.0;
    let windowRatio = wglr.windowWidth / wglr.windowHeight;

    if (gameRatio > windowRatio) {
        let viewportHeight = wglr.windowWidth / gameRatio;
        let offset = (wglr.windowHeight - viewportHeight) / 2;
        gl.viewport(0, offset, wglr.windowWidth, viewportHeight);
    }
    else {
        let viewportWidth = wglr.windowHeight * gameRatio;
        let offset = (wglr.windowWidth - viewportWidth) / 2;
        gl.viewport(offset, 0, viewportWidth, wglr.windowHeight);
    }

    gl.enable(gl.DEPTH_TEST);
    gl.enable(gl.CULL_FACE);
    gl.clearColor(0.09, 0.365, 0.545, 0.0);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.disable(gl.BLEND);
        gl.disable(gl.SCISSOR_TEST);
    //gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
}

function webglLoadTexture (id: number, width: number, height: number, pixelDataPtr: number): void {
    let gl: WebGLRenderingContext = wglr.glContext;
    let texture: WebGLTexture = gl.createTexture();
    if (wglr.textures.length != id) { throw 1; }
    wglr.textures.push(texture);

    wglr.textureSizes.push([width, height]);

    let pixelView = new Uint8Array(wglr.memoryView.buffer, pixelDataPtr, width * height * 4);

    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, width, height, 0, gl.RGBA, gl.UNSIGNED_BYTE, pixelView);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
}

function matrix4x4transpose (matrixBuffer: Float32Array) {
    var floatBuffer = new Float32Array(16)

    floatBuffer[0] = matrixBuffer[0];
    floatBuffer[1] = matrixBuffer[4];
    floatBuffer[2] = matrixBuffer[8];
    floatBuffer[3] = matrixBuffer[12];

    floatBuffer[4] = matrixBuffer[1];
    floatBuffer[5] = matrixBuffer[5];
    floatBuffer[6] = matrixBuffer[9];
    floatBuffer[7] = matrixBuffer[13];

    floatBuffer[8] = matrixBuffer[2];
    floatBuffer[9] = matrixBuffer[6];
    floatBuffer[10] = matrixBuffer[10];
    floatBuffer[11] = matrixBuffer[14];

    floatBuffer[12] = matrixBuffer[3];
    floatBuffer[13] = matrixBuffer[7];
    floatBuffer[14] = matrixBuffer[11];
    floatBuffer[15] = matrixBuffer[15]

    return floatBuffer;
}

function webglBasic3D (modelMatrixPtr: number, viewMatrixPtr: number, projMatrixPtr: number, 
                       textureID: number): void 
{
    let gl: WebGLRenderingContext = wglr.glContext;

    let program: WebGLProgram = wglr.basic3DShader.program;
    gl.useProgram(program);

    gl.bindBuffer(gl.ARRAY_BUFFER, wglr.basic3DVertexBuffer);
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, wglr.basic3DIndexBuffer);

    var positionLocation = gl.getAttribLocation(program, "position");
    gl.enableVertexAttribArray(positionLocation);
    gl.vertexAttribPointer(positionLocation, 3, gl.FLOAT, false, 20, 0);

    var texCoordLocation = gl.getAttribLocation(program, "texCoords");
    gl.enableVertexAttribArray(texCoordLocation);
    gl.vertexAttribPointer(texCoordLocation, 2, gl.FLOAT, false, 20, 12);

    //var normalLocation = gl.getAttribLocation(program, "normal");
    //gl.enableVertexAttribArray(normalLocation);
    //gl.vertexAttribPointer(normalLocation, 3, gl.FLOAT, false, 32, 20);

    // TODO(ebuchholz): figure out how to do this without making new array buffer view every frame
    var floatBuffer = new Float32Array(wglr.memoryView.buffer,
                                       modelMatrixPtr,
                                       16); //4x4 matrix
    var modelMatrixLocation = gl.getUniformLocation(program, "modelMatrix");
    gl.uniformMatrix4fv(modelMatrixLocation, false, matrix4x4transpose(floatBuffer));

    floatBuffer = new Float32Array(wglr.memoryView.buffer,
                                   viewMatrixPtr,
                                   16); //4x4 matrix
    var viewMatrixLocation = gl.getUniformLocation(program, "viewMatrix");
    gl.uniformMatrix4fv(viewMatrixLocation, false, matrix4x4transpose(floatBuffer));

    floatBuffer = new Float32Array(wglr.memoryView.buffer,
                                   projMatrixPtr,
                                   16); //4x4 matrix
    var projMatrixLocation = gl.getUniformLocation(program, "projMatrix");
    gl.uniformMatrix4fv(projMatrixLocation, false, matrix4x4transpose(floatBuffer));

    var texture = wglr.textures[textureID];
    var textureLocation = gl.getUniformLocation(program, "texture");
    gl.uniform1i(textureLocation, 0);
    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, texture);

    gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_SHORT, 0);
}

function webglSpriteBatchStart (): void {
    let gl: WebGLRenderingContext = wglr.glContext;

    let program: WebGLProgram = wglr.spriteShader.program;
    gl.useProgram(program);

    let screenWidthLoc: WebGLUniformLocation = gl.getUniformLocation(program, "screenDims.width");
    gl.uniform1f(screenWidthLoc, wglr.windowWidth);
    let screenHeightLoc: WebGLUniformLocation = gl.getUniformLocation(program, "screenDims.height");
    gl.uniform1f(screenHeightLoc, wglr.windowHeight);

    gl.bindBuffer(gl.ARRAY_BUFFER, wglr.spriteVertexBuffer);
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, wglr.spriteIndexBuffer);

    var positionLocation = gl.getAttribLocation(program, "pos");
    gl.enableVertexAttribArray(positionLocation);
    gl.vertexAttribPointer(positionLocation, 3, gl.FLOAT, false, 40, 0);

    var texCoordLocation = gl.getAttribLocation(program, "texCoords");
    gl.enableVertexAttribArray(texCoordLocation);
    gl.vertexAttribPointer(texCoordLocation, 2, gl.FLOAT, false, 40, 12);

    var colorCoordLocation = gl.getAttribLocation(program, "color");
    gl.enableVertexAttribArray(colorCoordLocation);
    gl.vertexAttribPointer(colorCoordLocation, 4, gl.FLOAT, false, 40, 20);

    var texIndexLocation = gl.getAttribLocation(program, "textureID");
    gl.enableVertexAttribArray(texIndexLocation);
    gl.vertexAttribPointer(texIndexLocation, 1, gl.FLOAT, false, 40, 36);
}

function webglSpriteBatchFlush (spriteDataPtr: number, numSpritesBatched: number, vertexSize: number, 
                                numSpritesDrawn: number, textureIDs: number, numTextures: number): void 
{
    let gl: WebGLRenderingContext = wglr.glContext;
    let program: WebGLProgram = wglr.spriteShader.program;

    let dataSize = numSpritesBatched * (vertexSize/4) * 4;
    let bufferOffset = numSpritesDrawn * vertexSize * 4;

    let newSpriteDataView = new Float32Array(wglr.memoryView.buffer, spriteDataPtr, dataSize);
    gl.bufferSubData(gl.ARRAY_BUFFER, bufferOffset, newSpriteDataView);

    let textureIDArrayView = new Uint32Array(wglr.memoryView.buffer, textureIDs, numTextures);
    for (let i = 0; i < numTextures; i++) {
        let texture = wglr.textures[textureIDArrayView[i]];

        gl.activeTexture(gl.TEXTURE0 + i);
        gl.bindTexture(gl.TEXTURE_2D, texture);

        var textureLocation = gl.getUniformLocation(program, "texture" + i);
        gl.uniform1i(textureLocation, i);

        var textureSizeLocation = gl.getUniformLocation(program, "texture" + i + "Size");
        gl.uniform2fv(textureSizeLocation, wglr.textureSizes[textureIDArrayView[i]]);
    }
    gl.drawElements(gl.TRIANGLES, numSpritesBatched * 6, gl.UNSIGNED_SHORT, numSpritesDrawn * 6);
}

